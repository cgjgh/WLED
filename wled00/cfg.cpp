#include "wled.h"
#include "wled_ethernet.h"

/*
 * Serializes and parses the cfg.json and wsec.json settings files, stored in internal FS.
 * The structure of the JSON is not to be considered an official API and may change without notice.
 */

//simple macro for ArduinoJSON's or syntax
#define CJSON(a,b) a = b | a

void getStringFromJson(char* dest, const char* src, size_t len) {
  if (src != nullptr) strlcpy(dest, src, len);
}

bool deserializeConfig(JsonObject doc, bool fromFS) {
  bool needsSave = false;
  //int rev_major = doc["rev"][0]; // 1
  //int rev_minor = doc["rev"][1]; // 0

  //long vid = doc[F("vid")]; // 2010020

  #ifdef WLED_USE_ETHERNET
  JsonObject ethernet = doc[F("eth")];
  CJSON(ethernetType, ethernet["type"]);
  // NOTE: Ethernet configuration takes priority over other use of pins
  WLED::instance().initEthernet();
  #endif

  JsonObject id = doc["id"];
  getStringFromJson(cmDNS, id[F("mdns")], 33);
  getStringFromJson(serverDescription, id[F("name")], 33);


  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientSSID, nw_ins_0[F("ssid")], 33);
  //int nw_ins_0_pskl = nw_ins_0[F("pskl")];
  //The WiFi PSK is normally not contained in the regular file for security reasons.
  //If it is present however, we will use it
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonArray nw_ins_0_ip = nw_ins_0["ip"];
  JsonArray nw_ins_0_gw = nw_ins_0["gw"];
  JsonArray nw_ins_0_sn = nw_ins_0["sn"];

  for (byte i = 0; i < 4; i++) {
    CJSON(staticIP[i], nw_ins_0_ip[i]);
    CJSON(staticGateway[i], nw_ins_0_gw[i]);
    CJSON(staticSubnet[i], nw_ins_0_sn[i]);
  }

  JsonObject ap = doc["ap"];
  getStringFromJson(apSSID, ap[F("ssid")], 33);
  getStringFromJson(apPass, ap["psk"] , 65); //normally not present due to security
  //int ap_pskl = ap[F("pskl")];

  CJSON(apChannel, ap[F("chan")]);
  if (apChannel > 13 || apChannel < 1) apChannel = 1;

  CJSON(apHide, ap[F("hide")]);
  if (apHide > 1) apHide = 1;

  CJSON(apBehavior, ap[F("behav")]);
  
  /*
  JsonArray ap_ip = ap["ip"];
  for (byte i = 0; i < 4; i++) {
    apIP[i] = ap_ip;
  }
  */

  noWifiSleep = doc[F("wifi")][F("sleep")] | !noWifiSleep; // inverted
  noWifiSleep = !noWifiSleep;
  //int wifi_phy = doc[F("wifi")][F("phy")]; //force phy mode n?

  JsonObject hw = doc[F("hw")];

  // read multiple button configuration
  JsonObject btn_obj = hw["btn"];
  int pull = -1; // trick for inverted setting
  CJSON(pull, btn_obj[F("pull")]);
  if (pull>=0) disablePullUp = pull;
  JsonArray hw_btn_ins = btn_obj[F("ins")];
  if (!hw_btn_ins.isNull()) {
    uint8_t s = 0;
    for (JsonObject btn : hw_btn_ins) {
      CJSON(buttonType[s], btn["type"]);
      int8_t pin = btn["pin"][0] | -1;
      if (pin > -1 && pinManager.allocatePin(pin, false, PinOwner::Button)) {
        btnPin[s] = pin;
      #ifdef ARDUINO_ARCH_ESP32
        // ESP32 only: check that analog button pin is a valid ADC gpio
        if (((buttonType[s] == BTN_TYPE_ANALOG) || (buttonType[s] == BTN_TYPE_ANALOG_INVERTED)) && (digitalPinToAnalogChannel(btnPin[s]) < 0)) 
        {
          // not an ADC analog pin
          DEBUG_PRINTF("PIN ALLOC error: GPIO%d for analog button #%d is not an analog pin!\n", btnPin[s], s);
          btnPin[s] = -1;
          pinManager.deallocatePin(pin,PinOwner::Button);
        } 
        else 
      #endif
        {
          if (disablePullUp) {
            pinMode(btnPin[s], INPUT);
          } else {
            #ifdef ESP32
            pinMode(btnPin[s], buttonType[s]==BTN_TYPE_PUSH_ACT_HIGH ? INPUT_PULLDOWN : INPUT_PULLUP);
            #else
            pinMode(btnPin[s], INPUT_PULLUP);
            #endif
          }
        }
      } else {
        btnPin[s] = -1;
      }
      JsonArray hw_btn_ins_0_macros = btn["macros"];
      CJSON(macroButton[s], hw_btn_ins_0_macros[0]);
      CJSON(macroLongPress[s],hw_btn_ins_0_macros[1]);
      CJSON(macroDoublePress[s], hw_btn_ins_0_macros[2]);
      if (++s >= WLED_MAX_BUTTONS) break; // max buttons reached
    }
    // clear remaining buttons
    for (; s<WLED_MAX_BUTTONS; s++) {
      btnPin[s]           = -1;
      buttonType[s]       = BTN_TYPE_NONE;
      macroButton[s]      = 0;
      macroLongPress[s]   = 0;
      macroDoublePress[s] = 0;
    }
  } else {
    // new install/missing configuration (button 0 has defaults)
    if (fromFS) {
      // relies upon only being called once with fromFS == true, which is currently true.
      uint8_t s = 0;
      if (pinManager.allocatePin(btnPin[0], false, PinOwner::Button)) { // initialized to #define value BTNPIN, or zero if not defined(!)
        ++s; // do not clear default button if allocated successfully 
      }
      for (; s<WLED_MAX_BUTTONS; s++) {
        btnPin[s]           = -1;
        buttonType[s]       = BTN_TYPE_NONE;
        macroButton[s]      = 0;
        macroLongPress[s]   = 0;
        macroDoublePress[s] = 0;
      }
    }
  }
  CJSON(touchThreshold,btn_obj[F("tt")]);
  CJSON(buttonPublishMqtt,btn_obj["mqtt"]);

  int hw_ir_pin = hw["ir"]["pin"] | -2; // 4
  if (hw_ir_pin > -2) {
    if (pinManager.allocatePin(hw_ir_pin, false, PinOwner::IR)) {
      irPin = hw_ir_pin;
    } else {
      irPin = -1;
    }
  }
  CJSON(irEnabled, hw["ir"]["type"]);

  JsonObject relay = hw[F("relay")];
  int hw_relay_pin = relay["pin"] | -2;
  if (hw_relay_pin > -2) {
    if (pinManager.allocatePin(hw_relay_pin,true, PinOwner::Relay)) {
      rlyPin = hw_relay_pin;
      pinMode(rlyPin, OUTPUT);
    } else {
      rlyPin = -1;
    }
  }
  if (relay.containsKey("rev")) {
    rlyMde = !relay["rev"];
  }


  JsonArray hw_if_i2c = hw[F("if")][F("i2c-pin")];
  CJSON(i2c_sda, hw_if_i2c[0]);
  CJSON(i2c_scl, hw_if_i2c[1]);
  PinManagerPinType i2c[2] = { { i2c_sda, true }, { i2c_scl, true } };
  if (i2c_scl >= 0 && i2c_sda >= 0 && pinManager.allocateMultiplePins(i2c, 2, PinOwner::HW_I2C)) {
    #ifdef ESP32
    Wire.setPins(i2c_sda, i2c_scl); // this will fail if Wire is initilised (Wire.begin() called prior)
    #endif
    Wire.begin();
  } else {
    i2c_sda = -1;
    i2c_scl = -1;
  }
  JsonArray hw_if_spi = hw[F("if")][F("spi-pin")];
  CJSON(spi_mosi, hw_if_spi[0]);
  CJSON(spi_sclk, hw_if_spi[1]);
  CJSON(spi_miso, hw_if_spi[2]);
  PinManagerPinType spi[3] = { { spi_mosi, true }, { spi_miso, true }, { spi_sclk, true } };
  if (spi_mosi >= 0 && spi_sclk >= 0 && pinManager.allocateMultiplePins(spi, 3, PinOwner::HW_SPI)) {
    #ifdef ESP32
    SPI.begin(spi_sclk, spi_miso, spi_mosi);  // SPI global uses VSPI on ESP32 and FSPI on C3, S3
    #else
    SPI.begin();
    #endif
  } else {
    spi_mosi = -1;
    spi_miso = -1;
    spi_sclk = -1;
  }

  //int hw_status_pin = hw[F("status")]["pin"]; // -1

   JsonObject interfaces = doc["if"];

#ifdef WLED_ENABLE_MQTT
  JsonObject if_mqtt = interfaces["mqtt"];
  CJSON(mqttEnabled, if_mqtt["en"]);
  getStringFromJson(mqttServer, if_mqtt[F("broker")], 33);
  CJSON(mqttPort, if_mqtt["port"]); // 1883
  getStringFromJson(mqttUser, if_mqtt[F("user")], 41);
  getStringFromJson(mqttPass, if_mqtt["psk"], 65); //normally not present due to security
  getStringFromJson(mqttClientID, if_mqtt[F("cid")], 41);

  getStringFromJson(mqttDeviceTopic, if_mqtt[F("topics")][F("device")], 33); // "wled/test"
  getStringFromJson(mqttGroupTopic, if_mqtt[F("topics")][F("group")], 33); // ""
#endif

  JsonObject if_ntp = interfaces[F("ntp")];
  CJSON(ntpEnabled, if_ntp["en"]);
  getStringFromJson(ntpServerName, if_ntp[F("host")], 33); // "1.wled.pool.ntp.org"
  CJSON(currentTimezone, if_ntp[F("tz")]);
  CJSON(utcOffsetSecs, if_ntp[F("offset")]);
  CJSON(useAMPM, if_ntp[F("ampm")]);
  CJSON(longitude, if_ntp[F("ln")]);
  CJSON(latitude, if_ntp[F("lt")]);


  JsonObject ota = doc["ota"];
  const char* pwd = ota["psk"]; //normally not present due to security

  bool pwdCorrect = !otaLock; //always allow access if ota not locked
  if (pwd != nullptr && strncmp(otaPass, pwd, 33) == 0) pwdCorrect = true;

  if (pwdCorrect) { //only accept these values from cfg.json if ota is unlocked (else from wsec.json)
    CJSON(otaLock, ota[F("lock")]);
    CJSON(wifiLock, ota[F("lock-wifi")]);
    CJSON(aOtaEnabled, ota[F("aota")]);
    getStringFromJson(otaPass, pwd, 33); //normally not present due to security
  }

  DEBUG_PRINTLN(F("Starting usermod config."));
  JsonObject usermods_settings = doc["um"];
  if (!usermods_settings.isNull()) {
    needsSave = !usermods.readFromConfig(usermods_settings);
  }

  if (fromFS) return needsSave;
  // if from /json/cfg
  doReboot = doc[F("rb")] | doReboot;
 // if (doInitBusses) return false; // no save needed, will do after bus init in wled.cpp loop
  return (doc["sv"] | true);
}

void deserializeConfigFromFS() {
  bool success = deserializeConfigSec();
  if (!success) { //if file does not exist, try reading from EEPROM
    #ifdef WLED_ADD_EEPROM_SUPPORT
    deEEPSettings();
    return;
    #endif
  }

  if (!requestJSONBufferLock(1)) return;

  DEBUG_PRINTLN(F("Reading settings from /cfg.json..."));

  success = readObjectFromFile("/cfg.json", nullptr, &doc);
  if (!success) { // if file does not exist, optionally try reading from EEPROM and then save defaults to FS
    releaseJSONBufferLock();
    #ifdef WLED_ADD_EEPROM_SUPPORT
    deEEPSettings();
    #endif

    // save default values to /cfg.json
    // call readFromConfig() with an empty object so that usermods can initialize to defaults prior to saving
    JsonObject empty = JsonObject();
    usermods.readFromConfig(empty);
    serializeConfig();
    // init Ethernet (in case default type is set at compile time)
    #ifdef WLED_USE_ETHERNET
    WLED::instance().initEthernet();
    #endif
    return;
  }

  // NOTE: This routine deserializes *and* applies the configuration
  //       Therefore, must also initialize ethernet from this function
  bool needsSave = deserializeConfig(doc.as<JsonObject>(), true);
  releaseJSONBufferLock();

  if (needsSave) serializeConfig(); // usermods required new parameters
}

void serializeConfig() {
  serializeConfigSec();

  DEBUG_PRINTLN(F("Writing settings to /cfg.json..."));

  if (!requestJSONBufferLock(2)) return;

  JsonArray rev = doc.createNestedArray("rev");
  rev.add(1); //major settings revision
  rev.add(0); //minor settings revision

  doc[F("vid")] = VERSION;

  JsonObject id = doc.createNestedObject("id");
  id[F("mdns")] = cmDNS;
  id[F("name")] = serverDescription;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0[F("ssid")] = clientSSID;
  nw_ins_0[F("pskl")] = strlen(clientPass);

  JsonArray nw_ins_0_ip = nw_ins_0.createNestedArray("ip");
  JsonArray nw_ins_0_gw = nw_ins_0.createNestedArray("gw");
  JsonArray nw_ins_0_sn = nw_ins_0.createNestedArray("sn");

  for (byte i = 0; i < 4; i++) {
    nw_ins_0_ip.add(staticIP[i]);
    nw_ins_0_gw.add(staticGateway[i]);
    nw_ins_0_sn.add(staticSubnet[i]);
  }

  JsonObject ap = doc.createNestedObject("ap");
  ap[F("ssid")] = apSSID;
  ap[F("pskl")] = strlen(apPass);
  ap[F("chan")] = apChannel;
  ap[F("hide")] = apHide;
  ap[F("behav")] = apBehavior;

  JsonArray ap_ip = ap.createNestedArray("ip");
  ap_ip.add(4);
  ap_ip.add(3);
  ap_ip.add(2);
  ap_ip.add(1);

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi[F("sleep")] = !noWifiSleep;
  //wifi[F("phy")] = 1;

  #ifdef WLED_USE_ETHERNET
  JsonObject ethernet = doc.createNestedObject("eth");
  ethernet["type"] = ethernetType;
  if (ethernetType != WLED_ETH_NONE && ethernetType < WLED_NUM_ETH_TYPES) {
    JsonArray pins = ethernet.createNestedArray("pin");
    for (uint8_t p=0; p<WLED_ETH_RSVD_PINS_COUNT; p++) pins.add(esp32_nonconfigurable_ethernet_pins[p].pin);
    if (ethernetBoards[ethernetType].eth_power>=0)     pins.add(ethernetBoards[ethernetType].eth_power);
    if (ethernetBoards[ethernetType].eth_mdc>=0)       pins.add(ethernetBoards[ethernetType].eth_mdc);
    if (ethernetBoards[ethernetType].eth_mdio>=0)      pins.add(ethernetBoards[ethernetType].eth_mdio);
    switch (ethernetBoards[ethernetType].eth_clk_mode) {
      case ETH_CLOCK_GPIO0_IN:
      case ETH_CLOCK_GPIO0_OUT:
        pins.add(0);
        break;
      case ETH_CLOCK_GPIO16_OUT:
        pins.add(16);
        break;
      case ETH_CLOCK_GPIO17_OUT:
        pins.add(17);
        break;
    }
  }
  #endif

   JsonObject hw = doc.createNestedObject("hw");

  // button(s)
  JsonObject hw_btn = hw.createNestedObject("btn");
  hw_btn["max"] = WLED_MAX_BUTTONS; // just information about max number of buttons (not actually used)
  hw_btn[F("pull")] = !disablePullUp;
  JsonArray hw_btn_ins = hw_btn.createNestedArray("ins");

  // configuration for all buttons
  for (uint8_t i=0; i<WLED_MAX_BUTTONS; i++) {
    JsonObject hw_btn_ins_0 = hw_btn_ins.createNestedObject();
    hw_btn_ins_0["type"] = buttonType[i];
    JsonArray hw_btn_ins_0_pin = hw_btn_ins_0.createNestedArray("pin");
    hw_btn_ins_0_pin.add(btnPin[i]);
    JsonArray hw_btn_ins_0_macros = hw_btn_ins_0.createNestedArray("macros");
    hw_btn_ins_0_macros.add(macroButton[i]);
    hw_btn_ins_0_macros.add(macroLongPress[i]);
    hw_btn_ins_0_macros.add(macroDoublePress[i]);
  }

  hw_btn[F("tt")] = touchThreshold;
  hw_btn["mqtt"] = buttonPublishMqtt;

  JsonObject hw_ir = hw.createNestedObject("ir");
  hw_ir["pin"] = irPin;
  hw_ir["type"] = irEnabled;  // the byte 'irEnabled' does contain the IR-Remote Type ( 0=disabled )

  JsonObject hw_relay = hw.createNestedObject(F("relay"));
  hw_relay["pin"] = rlyPin;
  hw_relay["rev"] = !rlyMde;

  hw[F("baud")] = serialBaud;

  JsonObject hw_if = hw.createNestedObject(F("if"));
  JsonArray hw_if_i2c = hw_if.createNestedArray("i2c-pin");
  hw_if_i2c.add(i2c_sda);
  hw_if_i2c.add(i2c_scl);
  JsonArray hw_if_spi = hw_if.createNestedArray("spi-pin");
  hw_if_spi.add(spi_mosi);
  hw_if_spi.add(spi_sclk);
  hw_if_spi.add(spi_miso);

  JsonObject hw_status = hw.createNestedObject("status");
  hw_status["pin"] = -1;

  

   JsonObject interfaces = doc.createNestedObject("if");

#ifdef WLED_ENABLE_MQTT
  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["en"] = mqttEnabled;
  if_mqtt[F("broker")] = mqttServer;
  if_mqtt["port"] = mqttPort;
  if_mqtt[F("user")] = mqttUser;
  if_mqtt[F("pskl")] = strlen(mqttPass);
  if_mqtt[F("cid")] = mqttClientID;

  JsonObject if_mqtt_topics = if_mqtt.createNestedObject(F("topics"));
  if_mqtt_topics[F("device")] = mqttDeviceTopic;
  if_mqtt_topics[F("group")] = mqttGroupTopic;
#endif

  JsonObject if_ntp = interfaces.createNestedObject("ntp");
  if_ntp["en"] = ntpEnabled;
  if_ntp[F("host")] = ntpServerName;
  if_ntp[F("tz")] = currentTimezone;
  if_ntp[F("offset")] = utcOffsetSecs;
  if_ntp[F("ampm")] = useAMPM;
  if_ntp[F("ln")] = longitude;
  if_ntp[F("lt")] = latitude;

  JsonObject ota = doc.createNestedObject("ota");
  ota[F("lock")] = otaLock;
  ota[F("lock-wifi")] = wifiLock;
  ota[F("pskl")] = strlen(otaPass);
  ota[F("aota")] = aOtaEnabled;


  JsonObject usermods_settings = doc.createNestedObject("um");
  usermods.addToConfig(usermods_settings);

  File f = WLED_FS.open("/cfg.json", "w");
  if (f) serializeJson(doc, f);
  f.close();
  releaseJSONBufferLock();

  doSerializeConfig = false;
}

//settings in /wsec.json, not accessible via webserver, for passwords and tokens
bool deserializeConfigSec() {
  DEBUG_PRINTLN(F("Reading settings from /wsec.json..."));

  if (!requestJSONBufferLock(3)) return false;

  bool success = readObjectFromFile("/wsec.json", nullptr, &doc);
  if (!success) {
    releaseJSONBufferLock();
    return false;
  }

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonObject ap = doc["ap"];
  getStringFromJson(apPass, ap["psk"] , 65);

#ifdef WLED_ENABLE_MQTT
  JsonObject interfaces = doc.createNestedObject("if");
  JsonObject if_mqtt = interfaces["mqtt"];
  getStringFromJson(mqttPass, if_mqtt["psk"], 65);
#endif

  getStringFromJson(settingsPIN, doc["pin"], 5);
  correctPIN = !strlen(settingsPIN);

  JsonObject ota = doc["ota"];
  getStringFromJson(otaPass, ota[F("pwd")], 33);
  CJSON(otaLock, ota[F("lock")]);
  CJSON(wifiLock, ota[F("lock-wifi")]);
  CJSON(aOtaEnabled, ota[F("aota")]);

  releaseJSONBufferLock();
  return true;
}

void serializeConfigSec() {
  DEBUG_PRINTLN(F("Writing settings to /wsec.json..."));

  if (!requestJSONBufferLock(4)) return;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0["psk"] = clientPass;

  JsonObject ap = doc.createNestedObject("ap");
  ap["psk"] = apPass;

#ifdef WLED_ENABLE_MQTT
  JsonObject interfaces = doc.createNestedObject("if");
  JsonObject if_mqtt = interfaces.createNestedObject("mqtt");
  if_mqtt["psk"] = mqttPass;
#endif

  doc["pin"] = settingsPIN;

  JsonObject ota = doc.createNestedObject("ota");
  ota[F("pwd")] = otaPass;
  ota[F("lock")] = otaLock;
  ota[F("lock-wifi")] = wifiLock;
  ota[F("aota")] = aOtaEnabled;

  File f = WLED_FS.open("/wsec.json", "w");
  if (f) serializeJson(doc, f);
  f.close();
  releaseJSONBufferLock();
}
