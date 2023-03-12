#ifndef WLED_FCN_DECLARE_H
#define WLED_FCN_DECLARE_H
#include <Arduino.h>

/*
 * All globally accessible functions are declared here
 */

//button.cpp
void shortPressAction(uint8_t b=0);
void longPressAction(uint8_t b=0);
void doublePressAction(uint8_t b=0);
bool isButtonPressed(uint8_t b=0);
void handleButton();
void handleIO();

//cfg.cpp
bool deserializeConfig(JsonObject doc, bool fromFS = false);
void deserializeConfigFromFS();
bool deserializeConfigSec();
void serializeConfig();
void serializeConfigSec();

template<typename DestType>
bool getJsonValue(const JsonVariant& element, DestType& destination) {
  if (element.isNull()) {
    return false;
  }

  destination = element.as<DestType>();
  return true;
}

template<typename DestType, typename DefaultType>
bool getJsonValue(const JsonVariant& element, DestType& destination, const DefaultType defaultValue) {
  if(!getJsonValue(element, destination)) {
    destination = defaultValue;
    return false;
  }

  return true;
}


//file.cpp
bool handleFileRead(AsyncWebServerRequest*, String path);
bool writeObjectToFileUsingId(const char* file, uint16_t id, JsonDocument* content);
bool writeObjectToFile(const char* file, const char* key, JsonDocument* content);
bool readObjectFromFileUsingId(const char* file, uint16_t id, JsonDocument* dest);
bool readObjectFromFile(const char* file, const char* key, JsonDocument* dest);
void updateFSInfo();
void closeFile();

//improv.cpp
void handleImprovPacket();
void sendImprovStateResponse(uint8_t state, bool error = false);
void sendImprovInfoResponse();
void sendImprovRPCResponse(uint8_t commandId);

//ir.cpp
void applyRepeatActions();
byte relativeChange(byte property, int8_t amount, byte lowerBoundary = 0, byte higherBoundary = 0xFF);
void decodeIR(uint32_t code);
void decodeIR24(uint32_t code);
void decodeIR24OLD(uint32_t code);
void decodeIR24CT(uint32_t code);
void decodeIR40(uint32_t code);
void decodeIR44(uint32_t code);
void decodeIR21(uint32_t code);
void decodeIR6(uint32_t code);
void decodeIR9(uint32_t code);
void decodeIRJson(uint32_t code);

void initIR();
void handleIR();

//json.cpp
#include "ESPAsyncWebServer.h"
#include "src/dependencies/json/ArduinoJson-v6.h"
#include "src/dependencies/json/AsyncJson-v6.h"

void deserializeSegment(JsonObject elem, byte it, byte presetId = 0);
bool deserializeState(JsonObject root);
void serializeState(JsonObject root, bool forPreset = false, bool includeBri = true, bool segmentBounds = true, bool selectedSegmentsOnly = false);
void serializeInfo(JsonObject root);
void serializeModeNames(JsonArray arr, const char *qstring);
void serializeModeData(JsonObject root);
void serveJson(AsyncWebServerRequest* request);

//mqtt.cpp
bool initMqtt();
void publishMqtt();

//ntp.cpp
void handleTime();
void handleNetworkTime();
void sendNTPPacket();
bool checkNTPResponse();    
void updateLocalTime();
void getTimeString(char* out);
void calculateSunriseAndSunset();
void setTimeFromAPI(uint32_t timein);

//set.cpp
bool isAsterisksOnly(const char* str, byte maxLen);
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage);
bool handleSet(AsyncWebServerRequest *request, const String& req, bool apply=true);

//network.cpp
int getSignalQuality(int rssi);
void WiFiEvent(WiFiEvent_t event);

//um_manager.cpp
typedef enum UM_Data_Types {
  UMT_BYTE = 0,
  UMT_UINT16,
  UMT_INT16,
  UMT_UINT32,
  UMT_INT32,
  UMT_FLOAT,
  UMT_DOUBLE,
  UMT_BYTE_ARR,
  UMT_UINT16_ARR,
  UMT_INT16_ARR,
  UMT_UINT32_ARR,
  UMT_INT32_ARR,
  UMT_FLOAT_ARR,
  UMT_DOUBLE_ARR
} um_types_t;
typedef struct UM_Exchange_Data {
  // should just use: size_t arr_size, void **arr_ptr, byte *ptr_type
  size_t       u_size;                 // size of u_data array
  um_types_t  *u_type;                 // array of data types
  void       **u_data;                 // array of pointers to data
  UM_Exchange_Data() {
    u_size = 0;
    u_type = nullptr;
    u_data = nullptr;
  }
  ~UM_Exchange_Data() {
    if (u_type) delete[] u_type;
    if (u_data) delete[] u_data;
  }
} um_data_t;
const unsigned int um_data_size = sizeof(um_data_t);  // 12 bytes

class Usermod {
  protected:
    um_data_t *um_data; // um_data should be allocated using new in (derived) Usermod's setup() or constructor
  public:
    Usermod() { um_data = nullptr; }
    virtual ~Usermod() { if (um_data) delete um_data; }
    virtual void setup() = 0; // pure virtual, has to be overriden
    virtual void loop() = 0;  // pure virtual, has to be overriden
    virtual void handleOverlayDraw() {}
    virtual bool handleButton(uint8_t b) { return false; }
    virtual bool getUMData(um_data_t **data) { if (data) *data = nullptr; return false; };
    virtual void connected() {}
    virtual void appendConfigData() {}
    virtual void addToJsonState(JsonObject& obj) {}
    virtual void addToJsonInfo(JsonObject& obj) {}
    virtual void readFromJsonState(JsonObject& obj) {}
    virtual void addToConfig(JsonObject& obj) {}
    virtual bool readFromConfig(JsonObject& obj) { return true; } // Note as of 2021-06 readFromConfig() now needs to return a bool, see usermod_v2_example.h
    virtual void onMqttConnect(bool sessionPresent) {}
    virtual bool onMqttMessage(char* topic, char* payload) { return false; }
    virtual void onUpdateBegin(bool) {}
    virtual uint16_t getId() {return USERMOD_ID_UNSPECIFIED;}
};

class UsermodManager {
  private:
    Usermod* ums[WLED_MAX_USERMODS];
    byte numMods = 0;

  public:
    void loop();
    void handleOverlayDraw();
    bool handleButton(uint8_t b);
    bool getUMData(um_data_t **um_data, uint8_t mod_id = USERMOD_ID_RESERVED); // USERMOD_ID_RESERVED will poll all usermods
    void setup();
    void connected();
    void appendConfigData();
    void addToJsonState(JsonObject& obj);
    void addToJsonInfo(JsonObject& obj);
    void readFromJsonState(JsonObject& obj);
    void addToConfig(JsonObject& obj);
    bool readFromConfig(JsonObject& obj);
    void onMqttConnect(bool sessionPresent);
    bool onMqttMessage(char* topic, char* payload);
    void onUpdateBegin(bool);
    bool add(Usermod* um);
    Usermod* lookup(uint16_t mod_id);
    byte getModCount() {return numMods;};
};

//usermods_list.cpp
void registerUsermods();

//usermod.cpp
void userSetup();
void userConnected();
void userLoop();
void stall_Change();


//util.cpp
int getNumVal(const String* req, uint16_t pos);
bool oappend(const char* txt); // append new c string to temp buffer efficiently
bool oappendi(int i);          // append new number to temp buffer efficiently
void sappend(char stype, const char* key, int val);
void sappends(char stype, const char* key, char* val);
void prepareHostname(char* hostname);
bool isAsterisksOnly(const char* str, byte maxLen);
bool requestJSONBufferLock(uint8_t module=255);
void releaseJSONBufferLock();


#ifdef WLED_ADD_EEPROM_SUPPORT
//wled_eeprom.cpp
void applyMacro(byte index);
void deEEP();
void deEEPSettings();
void clearEEPROM();
#endif

//wled_math.cpp
#ifndef WLED_USE_REAL_MATH
  template <typename T> T atan_t(T x);
  float cos_t(float phi);
  float sin_t(float x);
  float tan_t(float x);
  float acos_t(float x);
  float asin_t(float x);
  float floor_t(float x);
  float fmod_t(float num, float denom);
#else
  #include <math.h>
  #define sin_t sin
  #define cos_t cos
  #define tan_t tan
  #define asin_t asin
  #define acos_t acos
  #define atan_t atan
  #define fmod_t fmod
  #define floor_t floor
#endif

//wled_server.cpp
bool isIp(String str);
void createEditHandler(bool enable);
bool captivePortal(AsyncWebServerRequest *request);
void initServer();
void serveIndexOrWelcome(AsyncWebServerRequest *request);
void serveIndex(AsyncWebServerRequest* request);
String msgProcessor(const String& var);
void serveMessage(AsyncWebServerRequest* request, uint16_t code, const String& headl, const String& subl="", byte optionT=255);
String settingsProcessor(const String& var);
void serveSettings(AsyncWebServerRequest* request, bool post = false);
void serveSettingsJS(AsyncWebServerRequest* request);

//ws.cpp
void handleWs();
void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void sendDataWs(AsyncWebSocketClient * client = nullptr);

//xml.cpp
void XML_response(AsyncWebServerRequest *request, char* dest = nullptr);
void URL_response(AsyncWebServerRequest *request);
void getSettingsJS(byte subPage, char* dest);

#endif
