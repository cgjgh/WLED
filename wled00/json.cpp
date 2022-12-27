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
bool deserializeState(JsonObject root, byte callMode, byte presetId)
{
  // bool stateResponse = root[F("v")] | false;

  // #if defined(WLED_DEBUG) && defined(WLED_DEBUG_HOST)
  // netDebugEnabled = root[F("debug")] | netDebugEnabled;
  // #endif

  // bool onBefore = bri;
  // getVal(root["bri"], &bri);

  // bool on = root["on"] | (bri > 0);
  // if (!on != !bri) toggleOnOff();

  // if (root["on"].is<const char*>() && root["on"].as<const char*>()[0] == 't') {
  //   if (onBefore || !bri) toggleOnOff(); // do not toggle off again if just turned on by bri (makes e.g. "{"on":"t","bri":32}" work)
  // }

  // if (bri && !onBefore) { // unfreeze all segments when turning on
  //   for (size_t s=0; s < strip.getSegmentsNum(); s++) {
  //     strip.getSegment(s).freeze = false;
  //   }
  //   if (realtimeMode && !realtimeOverride && useMainSegmentOnly) { // keep live segment frozen if live
  //     strip.getMainSegment().freeze = true;
  //   }
  // }

  // int tr = -1;
  // if (!presetId || currentPlaylist < 0) { //do not apply transition time from preset if playlist active, as it would override playlist transition times
  //   tr = root[F("transition")] | -1;
  //   if (tr >= 0)
  //   {
  //     transitionDelay = tr;
  //     transitionDelay *= 100;
  //     transitionDelayTemp = transitionDelay;
  //   }
  // }

  // // temporary transition (applies only once)
  // tr = root[F("tt")] | -1;
  // if (tr >= 0)
  // {
  //   transitionDelayTemp = tr;
  //   transitionDelayTemp *= 100;
  //   jsonTransitionOnce = true;
  // }
  // strip.setTransition(transitionDelayTemp); // required here for color transitions to have correct duration

  // tr = root[F("tb")] | -1;
  // if (tr >= 0) strip.timebase = ((uint32_t)tr) - millis();

  // JsonObject nl       = root["nl"];
  // nightlightActive    = nl["on"]      | nightlightActive;
  // nightlightDelayMins = nl["dur"]     | nightlightDelayMins;
  // nightlightMode      = nl["mode"]    | nightlightMode;
  // nightlightTargetBri = nl[F("tbri")] | nightlightTargetBri;

  // JsonObject udpn      = root["udpn"];
  // notifyDirect         = udpn["send"] | notifyDirect;
  // receiveNotifications = udpn["recv"] | receiveNotifications;
  // if ((bool)udpn[F("nn")]) callMode = CALL_MODE_NO_NOTIFY; //send no notification just for this request

  // unsigned long timein = root["time"] | UINT32_MAX; //backup time source if NTP not synced
  // if (timein != UINT32_MAX) {
  //   setTimeFromAPI(timein);
  //   if (presetsModifiedTime == 0) presetsModifiedTime = timein;
  // }

  // doReboot = root[F("rb")] | doReboot;

  // // do not allow changing main segment while in realtime mode (may get odd results else)
  // if (!realtimeMode) strip.setMainSegmentId(root[F("mainseg")] | strip.getMainSegmentId()); // must be before realtimeLock() if "live"

  // realtimeOverride = root[F("lor")] | realtimeOverride;
  // if (realtimeOverride > 2) realtimeOverride = REALTIME_OVERRIDE_ALWAYS;
  // if (realtimeMode && useMainSegmentOnly) {
  //   strip.getMainSegment().freeze = !realtimeOverride;
  // }

  // if (root.containsKey("live")) {
  //   if (root["live"].as<bool>()) {
  //     transitionDelayTemp = 0;
  //     jsonTransitionOnce = true;
  //     realtimeLock(65000);
  //   } else {
  //     exitRealtime();
  //   }
  // }

  // int it = 0;
  // JsonVariant segVar = root["seg"];
  // if (segVar.is<JsonObject>())
  // {
  //   int id = segVar["id"] | -1;
  //   //if "seg" is not an array and ID not specified, apply to all selected/checked segments
  //   if (id < 0) {
  //     //apply all selected segments
  //     //bool didSet = false;
  //     for (size_t s = 0; s < strip.getSegmentsNum(); s++) {
  //       Segment &sg = strip.getSegment(s);
  //       if (sg.isSelected()) {
  //         deserializeSegment(segVar, s, presetId);
  //         //didSet = true;
  //       }
  //     }
  //     //TODO: not sure if it is good idea to change first active but unselected segment
  //     //if (!didSet) deserializeSegment(segVar, strip.getMainSegmentId(), presetId);
  //   } else {
  //     deserializeSegment(segVar, id, presetId); //apply only the segment with the specified ID
  //   }
  // } else {
  //   JsonArray segs = segVar.as<JsonArray>();
  //   for (JsonObject elem : segs) {
  //     deserializeSegment(elem, it, presetId);
  //     it++;
  //   }
  // }

  // usermods.readFromJsonState(root);

  // loadLedmap = root[F("ledmap")] | loadLedmap;

  // byte ps = root[F("psave")];
  // if (ps > 0 && ps < 251) savePreset(ps, nullptr, root);

  // ps = root[F("pdel")]; //deletion
  // if (ps > 0 && ps < 251) deletePreset(ps);

  // // HTTP API commands (must be handled before "ps")
  // const char* httpwin = root["win"];
  // if (httpwin) {
  //   String apireq = "win"; apireq += '&'; // reduce flash string usage
  //   apireq += httpwin;
  //   handleSet(nullptr, apireq, false);    // may set stateChanged
  // }

  // // applying preset (2 cases: a) API call includes all preset values ("pd"), b) API only specifies preset ID ("ps"))
  // byte presetToRestore = 0;
  // // a) already applied preset content (requires "seg" or "win" but will ignore the rest)
  // if (!root["pd"].isNull() && stateChanged) {
  //   currentPreset = root[F("pd")] | currentPreset;
  //   if (root["win"].isNull()) presetCycCurr = currentPreset;
  //   presetToRestore = currentPreset; // stateUpdated() will clear the preset, so we need to restore it after
  //   //unloadPlaylist(); // applying a preset unloads the playlist, may be needed here too?
  // } else if (!root["ps"].isNull()) {
  //   ps = presetCycCurr;
  //   if (root["win"].isNull() && getVal(root["ps"], &ps, 0, 0) && ps > 0 && ps < 251 && ps != currentPreset) {
  //     // b) preset ID only or preset that does not change state (use embedded cycling limits if they exist in getVal())
  //     presetCycCurr = ps;
  //     unloadPlaylist();          // applying a preset unloads the playlist
  //     applyPreset(ps, callMode); // async load from file system (only preset ID was specified)
  //     return stateResponse;
  //   }
  // }

  // JsonObject playlist = root[F("playlist")];
  // if (!playlist.isNull() && loadPlaylist(playlist, presetId)) {
  //   //do not notify here, because the first playlist entry will do
  //   if (root["on"].isNull()) callMode = CALL_MODE_NO_NOTIFY;
  //   else callMode = CALL_MODE_DIRECT_CHANGE;  // possible bugfix for playlist only containing HTTP API preset FX=~
  // }

  // stateUpdated(callMode);
  // if (presetToRestore) currentPreset = presetToRestore;

  // return stateResponse;
}

void serializeState(JsonObject root, bool forPreset, bool includeBri, bool segmentBounds, bool selectedSegmentsOnly)
{
  // if (includeBri) {
  //   root["on"] = (bri > 0);
  //   root["bri"] = briLast;
  //   root[F("transition")] = transitionDelay/100; //in 100ms
  // }

  // if (!forPreset) {
  //   if (errorFlag) {root[F("error")] = errorFlag; errorFlag = ERR_NONE;} //prevent error message to persist on screen

  //   root["ps"] = (currentPreset > 0) ? currentPreset : -1;
  //   root[F("pl")] = currentPlaylist;

  //   usermods.addToJsonState(root);

  //   JsonObject nl = root.createNestedObject("nl");
  //   nl["on"] = nightlightActive;
  //   nl["dur"] = nightlightDelayMins;
  //   nl["mode"] = nightlightMode;
  //   nl[F("tbri")] = nightlightTargetBri;
  //   if (nightlightActive) {
  //     nl[F("rem")] = (nightlightDelayMs - (millis() - nightlightStartTime)) / 1000; // seconds remaining
  //   } else {
  //     nl[F("rem")] = -1;
  //   }

  //   JsonObject udpn = root.createNestedObject("udpn");
  //   udpn["send"] = notifyDirect;
  //   udpn["recv"] = receiveNotifications;

  //   root[F("lor")] = realtimeOverride;
  // }

  // root[F("mainseg")] = strip.getMainSegmentId();

  // JsonArray seg = root.createNestedArray("seg");
  // for (size_t s = 0; s < strip.getMaxSegments(); s++) {
  //   if (s >= strip.getSegmentsNum()) {
  //     if (forPreset && segmentBounds && !selectedSegmentsOnly) { //disable segments not part of preset
  //       JsonObject seg0 = seg.createNestedObject();
  //       seg0["stop"] = 0;
  //       continue;
  //     } else
  //       break;
  //   }
  //   Segment &sg = strip.getSegment(s);
  //   if (forPreset && selectedSegmentsOnly && !sg.isSelected()) continue;
  //   if (sg.isActive()) {
  //     JsonObject seg0 = seg.createNestedObject();
  //     serializeSegment(seg0, sg, s, forPreset, segmentBounds);
  //   } else if (forPreset && segmentBounds) { //disable segments not part of preset
  //     JsonObject seg0 = seg.createNestedObject();
  //     seg0["stop"] = 0;
  //   }
  // }
}

void serializeInfo(JsonObject root)
{
  root[F("ver")] = versionString;
  root[F("vid")] = VERSION;
  //root[F("cn")] = WLED_CODENAME;

  // JsonObject leds = root.createNestedObject("leds");
  // leds[F("count")] = strip.getLengthTotal();
  // leds[F("pwr")] = strip.currentMilliamps;
  // leds["fps"] = strip.getFps();
  // leds[F("maxpwr")] = (strip.currentMilliamps)? strip.ablMilliampsMax : 0;
  // leds[F("maxseg")] = strip.getMaxSegments();
  // //leds[F("actseg")] = strip.getActiveSegmentsNum();
  // //leds[F("seglock")] = false; //might be used in the future to prevent modifications to segment config

  #ifndef WLED_DISABLE_2D
  if (strip.isMatrix) {
    JsonObject matrix = leds.createNestedObject("matrix");
    matrix["w"] = Segment::maxWidth;
    matrix["h"] = Segment::maxHeight;
  }
  #endif

  // uint8_t totalLC = 0;
  // JsonArray lcarr = leds.createNestedArray(F("seglc"));
  // size_t nSegs = strip.getSegmentsNum();
  // for (size_t s = 0; s < nSegs; s++) {
  //   if (!strip.getSegment(s).isActive()) continue;
  //   uint8_t lc = strip.getSegment(s).getLightCapabilities();
  //   totalLC |= lc;
  //   lcarr.add(lc);
  // }

  // leds["lc"] = totalLC;

  // leds[F("rgbw")] = strip.hasRGBWBus(); // deprecated, use info.leds.lc
  // leds[F("wv")]   = totalLC & 0x02;     // deprecated, true if white slider should be displayed for any segment
  // leds["cct"]     = totalLC & 0x04;     // deprecated, use info.leds.lc

  #ifdef WLED_DEBUG
  JsonArray i2c = root.createNestedArray(F("i2c"));
  i2c.add(i2c_sda);
  i2c.add(i2c_scl);
  JsonArray spi = root.createNestedArray(F("spi"));
  spi.add(spi_mosi);
  spi.add(spi_sclk);
  spi.add(spi_miso);
  #endif

  // root[F("str")] = syncToggleReceive;

  // root[F("name")] = serverDescription;
  // root[F("udpport")] = udpPort;
  // root["live"] = (bool)realtimeMode;
  // root[F("liveseg")] = useMainSegmentOnly ? strip.getMainSegmentId() : -1;  // if using main segment only for live

  // switch (realtimeMode) {
  //   case REALTIME_MODE_INACTIVE: root["lm"] = ""; break;
  //   case REALTIME_MODE_GENERIC:  root["lm"] = ""; break;
  //   case REALTIME_MODE_UDP:      root["lm"] = F("UDP"); break;
  //   case REALTIME_MODE_HYPERION: root["lm"] = F("Hyperion"); break;
  //   case REALTIME_MODE_E131:     root["lm"] = F("E1.31"); break;
  //   case REALTIME_MODE_ADALIGHT: root["lm"] = F("USB Adalight/TPM2"); break;
  //   case REALTIME_MODE_ARTNET:   root["lm"] = F("Art-Net"); break;
  //   case REALTIME_MODE_TPM2NET:  root["lm"] = F("tpm2.net"); break;
  //   case REALTIME_MODE_DDP:      root["lm"] = F("DDP"); break;
  // }

  // if (realtimeIP[0] == 0)
  // {
  //   root[F("lip")] = "";
  // } else {
  //   root[F("lip")] = realtimeIP.toString();
  // }

  // #ifdef WLED_ENABLE_WEBSOCKETS
  // root[F("ws")] = ws.count();
  // #else
  // root[F("ws")] = -1;
  // #endif

  // root[F("fxcount")] = strip.getModeCount();
  // root[F("palcount")] = strip.getPaletteCount();
  // root[F("cpalcount")] = strip.customPalettes.size(); //number of custom palettes


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
  #ifndef WLED_DISABLE_ALEXA
  os += 0x40;
  #endif
  #ifndef WLED_DISABLE_BLYNK
  os += 0x20;
  #endif
  #ifdef USERMOD_CRONIXIE
  os += 0x10;
  #endif
  #ifndef WLED_DISABLE_FILESYSTEM
  os += 0x08;
  #endif
  #ifndef WLED_DISABLE_HUESYNC
  os += 0x04;
  #endif
  #ifdef WLED_ENABLE_ADALIGHT
  os += 0x02;
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
  else if (url.indexOf("nodes") > 0) subJson = JSON_PATH_NODES;
  else if (url.indexOf("palx")  > 0) subJson = JSON_PATH_PALETTES;
  else if (url.indexOf("fxda")  > 0) subJson = JSON_PATH_FXDATA;
  else if (url.indexOf("net") > 0) subJson = JSON_PATH_NETWORKS;
  #ifdef WLED_ENABLE_JSONLIVE
  else if (url.indexOf("live")  > 0) {
    serveLiveLeds(request);
    return;
  }
  #endif
  
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

#ifdef WLED_ENABLE_JSONLIVE
#define MAX_LIVE_LEDS 180

bool serveLiveLeds(AsyncWebServerRequest* request, uint32_t wsClient)
{
  #ifdef WLED_ENABLE_WEBSOCKETS
  AsyncWebSocketClient * wsc = nullptr;
  if (!request) { //not HTTP, use Websockets
    wsc = ws.client(wsClient);
    if (!wsc || wsc->queueLength() > 0) return false; //only send if queue free
  }
  #endif

  uint16_t used = strip.getLengthTotal();
  uint16_t n = (used -1) /MAX_LIVE_LEDS +1; //only serve every n'th LED if count over MAX_LIVE_LEDS
  char buffer[2000];
  strcpy_P(buffer, PSTR("{\"leds\":["));
  obuf = buffer;
  olen = 9;

  for (size_t i= 0; i < used; i += n)
  {
    uint32_t c = strip.getPixelColor(i);
    uint8_t r = qadd8(W(c), R(c)); //add white channel to RGB channels as a simple RGBW -> RGB map
    uint8_t g = qadd8(W(c), G(c));
    uint8_t b = qadd8(W(c), B(c));
    olen += sprintf(obuf + olen, "\"%06X\",", RGBW32(r,g,b,0));
  }
  olen -= 1;
  oappend((const char*)F("],\"n\":"));
  oappendi(n);
  oappend("}");
  if (request) {
    request->send(200, "application/json", buffer);
  }
  #ifdef WLED_ENABLE_WEBSOCKETS
  else {
    wsc->text(obuf, olen);
  }
  #endif
  return true;
}
#endif
