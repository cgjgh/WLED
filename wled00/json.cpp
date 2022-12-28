#include "wled.h"


#define JSON_PATH_STATE      1
#define JSON_PATH_INFO       2
#define JSON_PATH_STATE_INFO 3
#define JSON_PATH_NODES      4
#define JSON_PATH_PALETTES   5
#define JSON_PATH_FXDATA     6
#define JSON_PATH_NETWORKS   7

/*
 * JSON API (De)serialization
 */

// deserializes WLED state (fileDoc points to doc object if called from web server)
// presetId is non-0 if called from handlePreset()
bool deserializeState(JsonObject root)
{
  bool stateResponse = root[F("v")] | false;

  #if defined(WLED_DEBUG) && defined(WLED_DEBUG_HOST)
  netDebugEnabled = root[F("debug")] | netDebugEnabled;
  #endif

  usermods.readFromJsonState(root);
  return stateResponse;
}

void serializeState(JsonObject root, bool forPreset, bool includeBri, bool segmentBounds, bool selectedSegmentsOnly)
{
    usermods.addToJsonState(root);
}

void serializeInfo(JsonObject root)
{
  root[F("ver")] = versionString;
  root[F("vid")] = VERSION;
  root[F("cn")] = WLED_CODENAME;

  #ifdef WLED_DEBUG
  JsonArray i2c = root.createNestedArray(F("i2c"));
  i2c.add(i2c_sda);
  i2c.add(i2c_scl);
  JsonArray spi = root.createNestedArray(F("spi"));
  spi.add(spi_mosi);
  spi.add(spi_sclk);
  spi.add(spi_miso);
  #endif

  JsonObject wifi_info = root.createNestedObject("wifi");
  wifi_info[F("bssid")] = WiFi.BSSIDstr();
  int qrssi = WiFi.RSSI();
  wifi_info[F("rssi")] = qrssi;
  wifi_info[F("signal")] = getSignalQuality(qrssi);
  wifi_info[F("channel")] = WiFi.channel();

  JsonObject fs_info = root.createNestedObject("fs");
  fs_info["u"] = fsBytesUsed / 1000;
  fs_info["t"] = fsBytesTotal / 1000;
  fs_info[F("pmt")] = presetsModifiedTime;

  #ifdef ARDUINO_ARCH_ESP32
  #ifdef WLED_DEBUG
    wifi_info[F("txPower")] = (int) WiFi.getTxPower();
    wifi_info[F("sleep")] = (bool) WiFi.getSleep();
  #endif
  #if !defined(CONFIG_IDF_TARGET_ESP32C2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)
    root[F("arch")] = "esp32";
  #else
    root[F("arch")] = ESP.getChipModel();
  #endif
  root[F("core")] = ESP.getSdkVersion();
  //root[F("maxalloc")] = ESP.getMaxAllocHeap();
  #ifdef WLED_DEBUG
    root[F("resetReason0")] = (int)rtc_get_reset_reason(0);
    root[F("resetReason1")] = (int)rtc_get_reset_reason(1);
  #endif
  root[F("lwip")] = 0; //deprecated
  #else
  root[F("arch")] = "esp8266";
  root[F("core")] = ESP.getCoreVersion();
  //root[F("maxalloc")] = ESP.getMaxFreeBlockSize();
  #ifdef WLED_DEBUG
    root[F("resetReason")] = (int)ESP.getResetInfoPtr()->reason;
  #endif
  root[F("lwip")] = LWIP_VERSION_MAJOR;
  #endif

  root[F("freeheap")] = ESP.getFreeHeap();
  #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)
  if (psramFound()) root[F("psram")] = ESP.getFreePsram();
  #endif
  root[F("uptime")] = millis()/1000 + rolloverMillis*4294967;

  usermods.addToJsonInfo(root);

  uint16_t os = 0;
  #ifdef WLED_DEBUG
  os  = 0x80;
    #ifdef WLED_DEBUG_HOST
    os |= 0x0100;
    if (!netDebugEnabled) os &= ~0x0080;
    #endif
  #endif
  #ifndef WLED_DISABLE_FILESYSTEM
  os += 0x08;
  #endif
  #ifndef WLED_DISABLE_OTA
  os += 0x01;
  #endif
  root[F("opt")] = os;

  root[F("brand")] = "WLED";
  root[F("product")] = F("FOSS");
  root["mac"] = escapedMac;
  char s[16] = "";
  if (Network.isConnected())
  {
    IPAddress localIP = Network.localIP();
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  }
  root["ip"] = s;
}


void serializeNetworks(JsonObject root)
{
  JsonArray networks = root.createNestedArray(F("networks"));
  int16_t status = WiFi.scanComplete();

  switch (status) {
    case WIFI_SCAN_FAILED:
      WiFi.scanNetworks(true);
      return;
    case WIFI_SCAN_RUNNING:
      return;
  }

  for (int i = 0; i < status; i++) {
    JsonObject node = networks.createNestedObject();
    node["ssid"]    = WiFi.SSID(i);
    node["rssi"]    = WiFi.RSSI(i);
    node["bssid"]   = WiFi.BSSIDstr(i);
    node["channel"] = WiFi.channel(i);
    node["enc"]     = WiFi.encryptionType(i);
  }

  WiFi.scanDelete();

  if (WiFi.scanComplete() == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
  }
}


void serveJson(AsyncWebServerRequest* request)
{
  byte subJson = 0;
  const String& url = request->url();
  if      (url.indexOf("state") > 0) subJson = JSON_PATH_STATE;
  else if (url.indexOf("info")  > 0) subJson = JSON_PATH_INFO;
  else if (url.indexOf("si")    > 0) subJson = JSON_PATH_STATE_INFO;
  else if (url.indexOf("net") > 0) subJson = JSON_PATH_NETWORKS;
  else if (url.indexOf("cfg") > 0 && handleFileRead(request, "/cfg.json")) {
    return;
  }
  else if (url.length() > 6) { //not just /json
    request->send(501, "application/json", F("{\"error\":\"Not implemented\"}"));
    return;
  }

  if (!requestJSONBufferLock(17)) {
    request->send(503, "application/json", F("{\"error\":3}"));
    return;
  }
  AsyncJsonResponse *response = new AsyncJsonResponse(&doc, subJson==6);

  JsonVariant lDoc = response->getRoot();

  switch (subJson)
  {
    case JSON_PATH_STATE:
      serializeState(lDoc); break;
    case JSON_PATH_INFO:
      serializeInfo(lDoc); break;
    case JSON_PATH_NETWORKS:
      serializeNetworks(lDoc); break;
    default: //all
      JsonObject state = lDoc.createNestedObject("state");
      serializeState(state);
      JsonObject info = lDoc.createNestedObject("info");
      serializeInfo(info);
      if (subJson != JSON_PATH_STATE_INFO)
      {
        
      }
      //lDoc["m"] = lDoc.memoryUsage(); // JSON buffer usage, for remote debugging
  }

  DEBUG_PRINTF("JSON buffer size: %u for request: %d\n", lDoc.memoryUsage(), subJson);

  response->setLength();
  request->send(response);
  releaseJSONBufferLock();
}
