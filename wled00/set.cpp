#include "wled.h"

/*
 * Receives client input
 */

//called upon POST settings form submit
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage)
{
  // PIN code request
  if (subPage == 252)
  {
    correctPIN = (strlen(settingsPIN)==0 || strncmp(settingsPIN, request->arg(F("PIN")).c_str(), 4)==0);
    lastEditTime = millis();
    return;
  }

  //0: menu 1: wifi 2: leds 3: ui 4: sync 5: time 6: sec 7: DMX 8: usermods 9: N/A 10: 2D
  if (subPage <1 || subPage >10 || !correctPIN) return;

  //WIFI SETTINGS
  if (subPage == 1)
  {
    strlcpy(clientSSID,request->arg(F("CS")).c_str(), 33);

    if (!isAsterisksOnly(request->arg(F("CP")).c_str(), 65)) strlcpy(clientPass, request->arg(F("CP")).c_str(), 65);

    strlcpy(cmDNS, request->arg(F("CM")).c_str(), 33);

    apBehavior = request->arg(F("AB")).toInt();
    strlcpy(apSSID, request->arg(F("AS")).c_str(), 33);
    apHide = request->hasArg(F("AH"));
    int passlen = request->arg(F("AP")).length();
    if (passlen == 0 || (passlen > 7 && !isAsterisksOnly(request->arg(F("AP")).c_str(), 65))) strlcpy(apPass, request->arg(F("AP")).c_str(), 65);
    int t = request->arg(F("AC")).toInt(); if (t > 0 && t < 14) apChannel = t;

    noWifiSleep = request->hasArg(F("WS"));

    #ifdef WLED_USE_ETHERNET
    ethernetType = request->arg(F("ETH")).toInt();
    WLED::instance().initEthernet();
    #endif

    char k[3]; k[2] = 0;
    for (int i = 0; i<4; i++)
    {
      k[1] = i+48;//ascii 0,1,2,3

      k[0] = 'I'; //static IP
      staticIP[i] = request->arg(k).toInt();

      k[0] = 'G'; //gateway
      staticGateway[i] = request->arg(k).toInt();

      k[0] = 'S'; //subnet
      staticSubnet[i] = request->arg(k).toInt();
    }
  }

  //UI
  if (subPage == 3)
  {
    strlcpy(serverDescription, request->arg(F("DS")).c_str(), 33);
  }

  //SYNC
  if (subPage == 4)
  {
    #ifdef WLED_ENABLE_MQTT
    mqttEnabled = request->hasArg(F("MQ"));
    strlcpy(mqttServer, request->arg(F("MS")).c_str(), 33);
    t = request->arg(F("MQPORT")).toInt();
    if (t > 0) mqttPort = t;
    strlcpy(mqttUser, request->arg(F("MQUSER")).c_str(), 41);
    if (!isAsterisksOnly(request->arg(F("MQPASS")).c_str(), 41)) strlcpy(mqttPass, request->arg(F("MQPASS")).c_str(), 65);
    strlcpy(mqttClientID, request->arg(F("MQCID")).c_str(), 41);
    strlcpy(mqttDeviceTopic, request->arg(F("MD")).c_str(), 33);
    strlcpy(mqttGroupTopic, request->arg(F("MG")).c_str(), 33);
    buttonPublishMqtt = request->hasArg(F("BM"));
    #endif

    // t = request->arg(F("BD")).toInt();
    // if (t >= 96 && t <= 15000) serialBaud = t;
    // updateBaudRate(serialBaud *100);
  }

  //TIME
  if (subPage == 5)
  {
    ntpEnabled = request->hasArg(F("NT"));
    strlcpy(ntpServerName, request->arg(F("NS")).c_str(), 33);
    useAMPM = !request->hasArg(F("CF"));
    currentTimezone = request->arg(F("TZ")).toInt();
    utcOffsetSecs = request->arg(F("UO")).toInt();

    //start ntp if not already connected
    if (ntpEnabled && WLED_CONNECTED && !ntpConnected) ntpConnected = ntpUdp.begin(ntpLocalPort);
    ntpLastSyncTime = 0; // force new NTP query

    longitude = request->arg(F("LN")).toFloat();
    latitude = request->arg(F("LT")).toFloat();
    // force a sunrise/sunset re-calculation
    calculateSunriseAndSunset(); 

    for (uint8_t i=0; i<WLED_MAX_BUTTONS; i++) {
      char mp[4] = "MP"; mp[2] = (i<10?48:55)+i; mp[3] = 0; // short
      char ml[4] = "ML"; ml[2] = (i<10?48:55)+i; ml[3] = 0; // long
      char md[4] = "MD"; md[2] = (i<10?48:55)+i; md[3] = 0; // double
      //if (!request->hasArg(mp)) break;
      macroButton[i] = request->arg(mp).toInt();      // these will default to 0 if not present
      macroLongPress[i] = request->arg(ml).toInt();
      macroDoublePress[i] = request->arg(md).toInt();
    }

  }

  //SECURITY
  if (subPage == 6)
  {
    if (request->hasArg(F("RS"))) //complete factory reset
    {
      WLED_FS.format();
      #ifdef WLED_ADD_EEPROM_SUPPORT
      clearEEPROM();
      #endif
      serveMessage(request, 200, F("All Settings erased."), F("Connect to WLED-AP to setup again"),255);
      doReboot = true;
    }

    if (request->hasArg(F("PIN"))) {
      const char *pin = request->arg(F("PIN")).c_str();
      uint8_t pinLen = strlen(pin);
      if (pinLen == 4 || pinLen == 0) {
        uint8_t numZeros = 0;
        for (uint8_t i = 0; i < pinLen; i++) numZeros += (pin[i] == '0');
        if (numZeros < pinLen || pinLen == 0) { // ignore 0000 input (placeholder)
          strlcpy(settingsPIN, pin, 5);
        }
        settingsPIN[4] = 0;
      }
    }

    bool pwdCorrect = !otaLock; //always allow access if ota not locked
    if (request->hasArg(F("OP")))
    {
      if (otaLock && strcmp(otaPass,request->arg(F("OP")).c_str()) == 0)
      {
        // brute force protection: do not unlock even if correct if last save was less than 3 seconds ago
        if (millis() - lastEditTime > 3000) pwdCorrect = true;
      }
      if (!otaLock && request->arg(F("OP")).length() > 0)
      {
        strlcpy(otaPass,request->arg(F("OP")).c_str(), 33); // set new OTA password
      }
    }

    if (pwdCorrect) //allow changes if correct pwd or no ota active
    {
      otaLock = request->hasArg(F("NO"));
      wifiLock = request->hasArg(F("OW"));
      aOtaEnabled = request->hasArg(F("AO"));
      //createEditHandler(correctPIN && !otaLock);
    }
  }

  //USERMODS
  if (subPage == 8)
  {
    if (!requestJSONBufferLock(5)) return;

    // global I2C & SPI pins
    int8_t hw_sda_pin  = !request->arg(F("SDA")).length() ? -1 : (int)request->arg(F("SDA")).toInt();
    int8_t hw_scl_pin  = !request->arg(F("SCL")).length() ? -1 : (int)request->arg(F("SCL")).toInt();
    #ifdef ESP8266
    // cannot change pins on ESP8266
    if (hw_sda_pin >= 0 && hw_sda_pin != HW_PIN_SDA) hw_sda_pin = HW_PIN_SDA;
    if (hw_scl_pin >= 0 && hw_scl_pin != HW_PIN_SCL) hw_scl_pin = HW_PIN_SCL;
    #endif
    PinManagerPinType i2c[2] = { { hw_sda_pin, true }, { hw_scl_pin, true } };
    if (hw_sda_pin >= 0 && hw_scl_pin >= 0 && pinManager.allocateMultiplePins(i2c, 2, PinOwner::HW_I2C)) {
      i2c_sda = hw_sda_pin;
      i2c_scl = hw_scl_pin;
      #ifdef ESP32
      Wire.setPins(i2c_sda, i2c_scl); // this will fail if Wire is initilised (Wire.begin() called)
      #endif
      Wire.begin();
    } else {
      // there is no Wire.end()
      DEBUG_PRINTLN(F("Could not allocate I2C pins."));
      uint8_t i2c[2] = { static_cast<uint8_t>(i2c_scl), static_cast<uint8_t>(i2c_sda) };
      pinManager.deallocateMultiplePins(i2c, 2, PinOwner::HW_I2C); // just in case deallocation of old pins
      i2c_sda = -1;
      i2c_scl = -1;
    }
    int8_t hw_mosi_pin = !request->arg(F("MOSI")).length() ? -1 : (int)request->arg(F("MOSI")).toInt();
    int8_t hw_miso_pin = !request->arg(F("MISO")).length() ? -1 : (int)request->arg(F("MISO")).toInt();
    int8_t hw_sclk_pin = !request->arg(F("SCLK")).length() ? -1 : (int)request->arg(F("SCLK")).toInt();
    #ifdef ESP8266
    // cannot change pins on ESP8266
    if (hw_mosi_pin >= 0 && hw_mosi_pin != HW_PIN_DATASPI)  hw_mosi_pin = HW_PIN_DATASPI;
    if (hw_miso_pin >= 0 && hw_miso_pin != HW_PIN_MISOSPI)  hw_mosi_pin = HW_PIN_MISOSPI;
    if (hw_sclk_pin >= 0 && hw_sclk_pin != HW_PIN_CLOCKSPI) hw_sclk_pin = HW_PIN_CLOCKSPI;
    #endif
    PinManagerPinType spi[3] = { { hw_mosi_pin, true }, { hw_miso_pin, true }, { hw_sclk_pin, true } };
    if (hw_mosi_pin >= 0 && hw_sclk_pin >= 0 && pinManager.allocateMultiplePins(spi, 3, PinOwner::HW_SPI)) {
      spi_mosi = hw_mosi_pin;
      spi_miso = hw_miso_pin;
      spi_sclk = hw_sclk_pin;
      // no bus re-initialisation as usermods do not get any notification
      //SPI.end();
      #ifdef ESP32
      //SPI.begin(spi_sclk, spi_miso, spi_mosi);
      #else
      //SPI.begin();
      #endif
    } else {
      //SPI.end();
      DEBUG_PRINTLN(F("Could not allocate SPI pins."));
      uint8_t spi[3] = { static_cast<uint8_t>(spi_mosi), static_cast<uint8_t>(spi_miso), static_cast<uint8_t>(spi_sclk) };
      pinManager.deallocateMultiplePins(spi, 3, PinOwner::HW_SPI); // just in case deallocation of old pins
      spi_mosi = -1;
      spi_miso = -1;
      spi_sclk = -1;
    }

    JsonObject um = doc.createNestedObject("um");

    size_t args = request->args();
    uint16_t j=0;
    for (size_t i=0; i<args; i++) {
      String name = request->argName(i);
      String value = request->arg(i);

      // POST request parameters are combined as <usermodname>_<usermodparameter>
      int umNameEnd = name.indexOf(":");
      if (umNameEnd<1) continue;  // parameter does not contain ":" or on 1st place -> wrong

      JsonObject mod = um[name.substring(0,umNameEnd)]; // get a usermod JSON object
      if (mod.isNull()) {
        mod = um.createNestedObject(name.substring(0,umNameEnd)); // if it does not exist create it
      }
      DEBUG_PRINT(name.substring(0,umNameEnd));
      DEBUG_PRINT(":");
      name = name.substring(umNameEnd+1); // remove mod name from string

      // if the resulting name still contains ":" this means nested object
      JsonObject subObj;
      int umSubObj = name.indexOf(":");
      DEBUG_PRINTF("(%d):",umSubObj);
      if (umSubObj>0) {
        subObj = mod[name.substring(0,umSubObj)];
        if (subObj.isNull())
          subObj = mod.createNestedObject(name.substring(0,umSubObj));
        name = name.substring(umSubObj+1); // remove nested object name from string
      } else {
        subObj = mod;
      }
      DEBUG_PRINT(name);

      // check if parameters represent array
      if (name.endsWith("[]")) {
        name.replace("[]","");
        value.replace(",",".");      // just in case conversion
        if (!subObj[name].is<JsonArray>()) {
          JsonArray ar = subObj.createNestedArray(name);
          if (value.indexOf(".") >= 0) ar.add(value.toFloat());  // we do have a float
          else                         ar.add(value.toInt());    // we may have an int
          j=0;
        } else {
          if (value.indexOf(".") >= 0) subObj[name].add(value.toFloat());  // we do have a float
          else                         subObj[name].add(value.toInt());    // we may have an int
          j++;
        }
        DEBUG_PRINT("[");
        DEBUG_PRINT(j);
        DEBUG_PRINT("] = ");
        DEBUG_PRINTLN(value);
      } else {
        // we are using a hidden field with the same name as our parameter (!before the actual parameter!)
        // to describe the type of parameter (text,float,int), for boolean patameters the first field contains "off"
        // so checkboxes have one or two fields (first is always "false", existence of second depends on checkmark and may be "true")
        if (subObj[name].isNull()) {
          // the first occurence of the field describes the parameter type (used in next loop)
          if (value == "false") subObj[name] = false; // checkboxes may have only one field
          else                  subObj[name] = value;
        } else {
          String type = subObj[name].as<String>();  // get previously stored value as a type
          if (subObj[name].is<bool>())   subObj[name] = true;   // checkbox/boolean
          else if (type == "number") {
            value.replace(",",".");      // just in case conversion
            if (value.indexOf(".") >= 0) subObj[name] = value.toFloat();  // we do have a float
            else                         subObj[name] = value.toInt();    // we may have an int
          } else if (type == "int")      subObj[name] = value.toInt();
          else                           subObj[name] = value;  // text fields
        }
        DEBUG_PRINT(" = ");
        DEBUG_PRINTLN(value);
      }
    }
    usermods.readFromConfig(um);  // force change of usermod parameters
    DEBUG_PRINTLN(F("Done re-init usermods."));
    releaseJSONBufferLock();
  }

  lastEditTime = millis();
  if (subPage != 2 && !doReboot) doSerializeConfig = true; //serializeConfig(); //do not save if factory reset or LED settings (which are saved after LED re-init)
 
}


//HTTP API request parser
bool handleSet(AsyncWebServerRequest *request, const String& req, bool apply)
{

  return true;
}
