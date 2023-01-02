#ifndef WLED_H
#define WLED_H
/*
   Main sketch, global variable declarations
   @title WLED project sketch
   @version 0.14.0-b1
   @author Christian Schwinne
 */

// version code in format yymmddb (b = daily build)
#define VERSION 2212222

//uncomment this if you have a "my_config.h" file you'd like to use
//#define WLED_USE_MY_CONFIG

// ESP8266-01 (blue) got too little storage space to work with WLED. 0.10.2 is the last release supporting this unit.

// ESP8266-01 (black) has 1MB flash and can thus fit the whole program, although OTA update is not possible. Use 1M(128K SPIFFS).
// 2-step OTA may still be possible: https://github.com/Aircoookie/WLED/issues/2040#issuecomment-981111096
// Uncomment some of the following lines to disable features:
// Alternatively, with platformio pass your chosen flags to your custom build target in platformio_override.ini

// You are required to disable over-the-air updates:
//#define WLED_DISABLE_OTA         // saves 14kb

// You can choose some of these features to disable:
#define WLED_DISABLE_ALEXA       // saves 11kb
#define WLED_DISABLE_BLYNK       // saves 6kb
#define WLED_DISABLE_HUESYNC     // saves 4kb
#define WLED_DISABLE_INFRARED    // saves 12kb, there is no pin left for this on ESP8266-01
#define WLED_DISABLE_MQTT
#define WLED_DISABLE_WEBSOCKETS
#define WLED_DISABLE_LOXONE
#define WLED_DISABLE_2D

#ifndef WLED_DISABLE_MQTT
  #define WLED_ENABLE_MQTT         // saves 12kb
#endif
//#define WLED_ENABLE_ADALIGHT       // saves 500b only (uses GPIO3 (RX) for serial)
//#define WLED_ENABLE_DMX          // uses 3.5kb (use LEDPIN other than 2)
//#define WLED_ENABLE_JSONLIVE     // peek LED output via /json/live (WS binary peek is always enabled)

#ifndef WLED_DISABLE_LOXONE
  #define WLED_ENABLE_LOXONE       // uses 1.2kb
#endif
#ifndef WLED_DISABLE_WEBSOCKETS
  #define WLED_ENABLE_WEBSOCKETS
#endif

#define WLED_ENABLE_FS_EDITOR      // enable /edit page for editing FS content. Will also be disabled with OTA lock

// to toggle usb serial debug (un)comment the following line
#define WLED_DEBUG

// filesystem specific debugging
#define WLED_DEBUG_FS

#ifndef WLED_WATCHDOG_TIMEOUT
  // 3 seconds should be enough to detect a lockup
  // define WLED_WATCHDOG_TIMEOUT=0 to disable watchdog, default
  #define WLED_WATCHDOG_TIMEOUT 0
#endif

//optionally disable brownout detector on ESP32.
//This is generally a terrible idea, but improves boot success on boards with a 3.3v regulator + cap setup that can't provide 400mA peaks
//#define WLED_DISABLE_BROWNOUT_DET

// Library inclusions.
#include <Arduino.h>
#include <NoDelay.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESPAsyncTCP.h>
  #include <LittleFS.h>
  extern "C"
  {
  #include <user_interface.h>
  }
#else // ESP32
  #include <HardwareSerial.h>  // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)
  #include <WiFi.h>
  #include <ETH.h>
  #include "esp_wifi.h"
  #include <ESPmDNS.h>
  #include <AsyncTCP.h>
  #if LOROL_LITTLEFS
    #ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
      #define CONFIG_LITTLEFS_FOR_IDF_3_2
    #endif
    #include <LITTLEFS.h>
  #else
    #include <LittleFS.h>
  #endif
  #include "esp_task_wdt.h"
#endif
#include <Wire.h>
#include <SPI.h>

#include "src/dependencies/network/Network.h"

#ifdef WLED_USE_MY_CONFIG
  #include "my_config.h"
#endif

#ifdef WLED_DEBUG_HOST
#include "net_debug.h"
#endif

#include <ESPAsyncWebServer.h>
#ifdef WLED_ADD_EEPROM_SUPPORT
  #include <EEPROM.h>
#endif
#include <WiFiUdp.h>
#include <DNSServer.h>
#ifndef WLED_DISABLE_OTA
  #define NO_OTA_PORT
  #include <ArduinoOTA.h>
#endif
#include <SPIFFSEditor.h>
#include "src/dependencies/time/TimeLib.h"
#include "src/dependencies/timezone/Timezone.h"
#include "src/dependencies/toki/Toki.h"


#include "src/dependencies/async-mqtt-client/AsyncMqttClient.h"

#define ARDUINOJSON_DECODE_UNICODE 0
#include "src/dependencies/json/AsyncJson-v6.h"
#include "src/dependencies/json/ArduinoJson-v6.h"

// ESP32-WROVER features SPI RAM (aka PSRAM) which can be allocated using ps_malloc()
// we can create custom PSRAMDynamicJsonDocument to use such feature (replacing DynamicJsonDocument)
// The following is a construct to enable code to compile without it.
// There is a code thet will still not use PSRAM though:
//    AsyncJsonResponse is a derived class that implements DynamicJsonDocument (AsyncJson-v6.h)
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)
struct PSRAM_Allocator {
  void* allocate(size_t size) {
    if (psramFound()) return ps_malloc(size); // use PSRAM if it exists
    else              return malloc(size);    // fallback
  }
  void deallocate(void* pointer) {
    free(pointer);
  }
};
using PSRAMDynamicJsonDocument = BasicJsonDocument<PSRAM_Allocator>;
#else
#define PSRAMDynamicJsonDocument DynamicJsonDocument
#endif

#include "const.h"
#include "fcn_declare.h"
#include "pin_manager.h"

#ifndef CLIENT_SSID
  #define CLIENT_SSID DEFAULT_CLIENT_SSID
#endif

#ifndef CLIENT_PASS
  #define CLIENT_PASS ""
#endif

#if defined(WLED_AP_PASS) && !defined(WLED_AP_SSID)
  #error WLED_AP_PASS is defined but WLED_AP_SSID is still the default. \
         Please change WLED_AP_SSID to something unique.
#endif

#ifndef WLED_AP_SSID
  #define WLED_AP_SSID DEFAULT_AP_SSID
#endif

#ifndef WLED_AP_PASS
  #define WLED_AP_PASS DEFAULT_AP_PASS
#endif
#ifndef WLED_HTTP_USER
  #define WLED_HTTP_USER DEFAULT_HTTP_USER
#endif

#ifndef WLED_HTTP_PASS
  #define WLED_HTTP_PASS DEFAULT_HTTP_PASS
#endif

#ifndef SPIFFS_EDITOR_AIRCOOOKIE
  #error You are not using the Aircoookie fork of the ESPAsyncWebserver library.\
  Using upstream puts your WiFi password at risk of being served by the filesystem.\
  Comment out this error message to build regardless.
#endif

#ifndef WLED_DISABLE_INFRARED
  #include <IRremoteESP8266.h>
  #include <IRrecv.h>
  #include <IRutils.h>
#endif

//Filesystem to use for preset and config files. SPIFFS or LittleFS on ESP8266, SPIFFS only on ESP32 (now using LITTLEFS port by lorol)
#ifdef ESP8266
  #define WLED_FS LittleFS
#else
  #if LOROL_LITTLEFS
    #define WLED_FS LITTLEFS
  #else
    #define WLED_FS LittleFS
  #endif
#endif

// GLOBAL VARIABLES
// both declared and defined in header (solution from http://www.keil.com/support/docs/1868.htm)
//
//e.g. byte test = 2 becomes WLED_GLOBAL byte test _INIT(2);
//     int arr[]{0,1,2} becomes WLED_GLOBAL int arr[] _INIT_N(({0,1,2}));

#ifndef WLED_DEFINE_GLOBAL_VARS
# define WLED_GLOBAL extern
# define _INIT(x)
# define _INIT_N(x)
#else
# define WLED_GLOBAL
# define _INIT(x) = x

//needed to ignore commas in array definitions
#define UNPACK( ... ) __VA_ARGS__
# define _INIT_N(x) UNPACK x
#endif

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

#ifndef WLED_VERSION
  #define WLED_VERSION "dev"
#endif

// Global Variable definitions
WLED_GLOBAL char versionString[] _INIT(TOSTRING(WLED_VERSION));
#define WLED_CODENAME "Hoshi"

// AP and OTA default passwords (for maximum security change them!)
WLED_GLOBAL char apPass[65]  _INIT(WLED_AP_PASS);
WLED_GLOBAL char otaPass[33] _INIT(DEFAULT_OTA_PASS);

// Hardware and pin config
#ifndef BTNPIN
WLED_GLOBAL int8_t btnPin[WLED_MAX_BUTTONS] _INIT({0});
#else
WLED_GLOBAL int8_t btnPin[WLED_MAX_BUTTONS] _INIT({BTNPIN});
#endif
#ifndef RLYPIN
WLED_GLOBAL int8_t rlyPin _INIT(-1);
#else
WLED_GLOBAL int8_t rlyPin _INIT(RLYPIN);
#endif
//Relay mode (1 = active high, 0 = active low, flipped in cfg.json)
#ifndef RLYMDE
WLED_GLOBAL bool rlyMde _INIT(true);
#else
WLED_GLOBAL bool rlyMde _INIT(RLYMDE);
#endif
#ifndef IRPIN
WLED_GLOBAL int8_t irPin _INIT(-1);
#else
WLED_GLOBAL int8_t irPin _INIT(IRPIN);
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S2) || (defined(RX) && defined(TX))
  // use RX/TX as set by the framework - these boards do _not_ have RX=3 and TX=1
  constexpr uint8_t hardwareRX = RX;
  constexpr uint8_t hardwareTX = TX;
#else
  // use defaults for RX/TX
  constexpr uint8_t hardwareRX = 3;
  constexpr uint8_t hardwareTX = 1;
#endif

//WLED_GLOBAL byte presetToApply _INIT(0);

WLED_GLOBAL char ntpServerName[33] _INIT("0.wled.pool.ntp.org");   // NTP server to use

// WiFi CONFIG (all these can be changed via web UI, no need to set them here)
WLED_GLOBAL char clientSSID[33] _INIT(CLIENT_SSID);
WLED_GLOBAL char clientPass[65] _INIT(CLIENT_PASS);
WLED_GLOBAL char cmDNS[33] _INIT("x");                             // mDNS address (placeholder, is replaced by wledXXXXXX.local)
WLED_GLOBAL char apSSID[33] _INIT("");                             // AP off by default (unless setup)
WLED_GLOBAL byte apChannel _INIT(1);                               // 2.4GHz WiFi AP channel (1-13)
WLED_GLOBAL byte apHide    _INIT(0);                               // hidden AP SSID
WLED_GLOBAL byte apBehavior _INIT(AP_BEHAVIOR_BOOT_NO_CONN);       // access point opens when no connection after boot by default
WLED_GLOBAL IPAddress staticIP      _INIT_N(((  0,   0,  0,  0))); // static IP of ESP
WLED_GLOBAL IPAddress staticGateway _INIT_N(((  0,   0,  0,  0))); // gateway (router) IP
WLED_GLOBAL IPAddress staticSubnet  _INIT_N(((255, 255, 255, 0))); // most common subnet in home networks
#ifdef ARDUINO_ARCH_ESP32
WLED_GLOBAL bool noWifiSleep _INIT(true);                          // disabling modem sleep modes will increase heat output and power usage, but may help with connection issues
#else
WLED_GLOBAL bool noWifiSleep _INIT(false);
#endif

#ifdef WLED_USE_ETHERNET
  #ifdef WLED_ETH_DEFAULT                                          // default ethernet board type if specified
    WLED_GLOBAL int ethernetType _INIT(WLED_ETH_DEFAULT);          // ethernet board type
  #else
    WLED_GLOBAL int ethernetType _INIT(WLED_ETH_NONE);             // use none for ethernet board type if default not defined
  #endif
#endif

// User Interface CONFIG
#ifndef SERVERNAME
WLED_GLOBAL char serverDescription[33] _INIT("WLED");  // Name of module - use default
#else
WLED_GLOBAL char serverDescription[33] _INIT(SERVERNAME);  // use predefined name
#endif
WLED_GLOBAL byte cacheInvalidate       _INIT(0);       // used to invalidate browser cache when switching from regular to simplified UI

// Sync CONFIG
WLED_GLOBAL byte buttonType[WLED_MAX_BUTTONS]  _INIT({BTN_TYPE_PUSH});
#if defined(IRTYPE) && defined(IRPIN)
WLED_GLOBAL byte irEnabled      _INIT(IRTYPE); // Infrared receiver
#else
WLED_GLOBAL byte irEnabled      _INIT(0);     // Infrared receiver disabled
#endif

WLED_GLOBAL bool mqttEnabled _INIT(false);
WLED_GLOBAL char mqttDeviceTopic[33] _INIT("");            // main MQTT topic (individual per device, default is wled/mac)
WLED_GLOBAL char mqttGroupTopic[33] _INIT("wled/all");     // second MQTT topic (for example to group devices)
WLED_GLOBAL char mqttServer[33] _INIT("");                 // both domains and IPs should work (no SSL)
WLED_GLOBAL char mqttUser[41] _INIT("");                   // optional: username for MQTT auth
WLED_GLOBAL char mqttPass[65] _INIT("");                   // optional: password for MQTT auth
WLED_GLOBAL char mqttClientID[41] _INIT("");               // override the client ID
WLED_GLOBAL uint16_t mqttPort _INIT(1883);


WLED_GLOBAL uint16_t serialBaud _INIT(1152); // serial baud rate, multiply by 100

// Time CONFIG
WLED_GLOBAL bool ntpEnabled _INIT(false);    // get internet time. Only required if you use clock overlays or time-activated macros
WLED_GLOBAL bool useAMPM _INIT(false);       // 12h/24h clock format
WLED_GLOBAL byte currentTimezone _INIT(0);   // Timezone ID. Refer to timezones array in wled10_ntp.ino
WLED_GLOBAL int utcOffsetSecs _INIT(0);      // Seconds to offset from UTC before timzone calculation

WLED_GLOBAL byte macroButton[WLED_MAX_BUTTONS]        _INIT({0});
WLED_GLOBAL byte macroLongPress[WLED_MAX_BUTTONS]     _INIT({0});
WLED_GLOBAL byte macroDoublePress[WLED_MAX_BUTTONS]   _INIT({0});

// Security CONFIG
WLED_GLOBAL bool otaLock     _INIT(false);  // prevents OTA firmware updates without password. ALWAYS enable if system exposed to any public networks
WLED_GLOBAL bool wifiLock    _INIT(false);  // prevents access to WiFi settings when OTA lock is enabled
WLED_GLOBAL bool aOtaEnabled _INIT(true);   // ArduinoOTA allows easy updates directly from the IDE. Careful, it does not auto-disable when OTA lock is on
WLED_GLOBAL char settingsPIN[5] _INIT("");  // PIN for settings pages
WLED_GLOBAL bool correctPIN     _INIT(true);
WLED_GLOBAL unsigned long lastEditTime _INIT(0);

WLED_GLOBAL uint16_t userVar0 _INIT(0), userVar1 _INIT(0); //available for use in usermod

// internal global variable declarations
// wifi
WLED_GLOBAL bool apActive _INIT(false);
WLED_GLOBAL bool forceReconnect _INIT(false);
WLED_GLOBAL uint32_t lastReconnectAttempt _INIT(0);
WLED_GLOBAL bool interfacesInited _INIT(false);
WLED_GLOBAL bool wasConnected _INIT(false);

// button
WLED_GLOBAL bool buttonPublishMqtt                            _INIT(false);
WLED_GLOBAL bool buttonPressedBefore[WLED_MAX_BUTTONS]        _INIT({false});
WLED_GLOBAL bool buttonLongPressed[WLED_MAX_BUTTONS]          _INIT({false});
WLED_GLOBAL unsigned long buttonPressedTime[WLED_MAX_BUTTONS] _INIT({0});
WLED_GLOBAL unsigned long buttonWaitTime[WLED_MAX_BUTTONS]    _INIT({0});
WLED_GLOBAL bool disablePullUp                                _INIT(false);
WLED_GLOBAL byte touchThreshold                               _INIT(TOUCH_THRESHOLD);

// network
WLED_GLOBAL bool udpConnected _INIT(false), udp2Connected _INIT(false), udpRgbConnected _INIT(false);

// ui style
WLED_GLOBAL bool showWelcomePage _INIT(false);

//improv
WLED_GLOBAL byte improvActive _INIT(0); //0: no improv packet received, 1: improv active, 2: provisioning
WLED_GLOBAL byte improvError _INIT(0);

// mqtt
WLED_GLOBAL unsigned long lastMqttReconnectAttempt _INIT(0);
WLED_GLOBAL unsigned long lastInterfaceUpdate _INIT(0);
WLED_GLOBAL char mqttStatusTopic[40] _INIT("");        // this must be global because of async handlers

// alexa udp
WLED_GLOBAL String escapedMac;

// dns server
WLED_GLOBAL DNSServer dnsServer;

// network time
WLED_GLOBAL bool ntpConnected _INIT(false);
WLED_GLOBAL time_t localTime _INIT(0);
WLED_GLOBAL unsigned long ntpLastSyncTime _INIT(999000000L);
WLED_GLOBAL unsigned long ntpPacketSentTime _INIT(999000000L);
WLED_GLOBAL IPAddress ntpServerIP;
WLED_GLOBAL uint16_t ntpLocalPort _INIT(2390);
WLED_GLOBAL uint16_t rolloverMillis _INIT(0);
WLED_GLOBAL float longitude _INIT(0.0);
WLED_GLOBAL float latitude _INIT(0.0);
WLED_GLOBAL time_t sunrise _INIT(0);
WLED_GLOBAL time_t sunset _INIT(0);
WLED_GLOBAL Toki toki _INIT(Toki());

// Temp buffer
WLED_GLOBAL char* obuf;
WLED_GLOBAL uint16_t olen _INIT(0);

// General filesystem
WLED_GLOBAL size_t fsBytesUsed _INIT(0);
WLED_GLOBAL size_t fsBytesTotal _INIT(0);
WLED_GLOBAL unsigned long presetsModifiedTime _INIT(0L);
WLED_GLOBAL JsonDocument* fileDoc;
WLED_GLOBAL bool doCloseFile _INIT(false);

// presets
WLED_GLOBAL byte currentPreset _INIT(0);

WLED_GLOBAL byte errorFlag _INIT(0);

WLED_GLOBAL String messageHead, messageSub;
WLED_GLOBAL byte optionType;

WLED_GLOBAL bool doSerializeConfig _INIT(false);        // flag to initiate saving of config
WLED_GLOBAL bool doReboot          _INIT(false);        // flag to initiate reboot from async handlers
WLED_GLOBAL bool doPublishMqtt     _INIT(false);

// relay

WLED_GLOBAL byte ledState _INIT(LOW);
WLED_GLOBAL byte cmd _INIT(0);
WLED_GLOBAL byte blinkCounter _INIT(0);

// status led
#if defined(STATUSLED)
WLED_GLOBAL unsigned long ledStatusLastMillis _INIT(0);
WLED_GLOBAL uint8_t ledStatusType _INIT(0); // current status type - corresponds to number of blinks per second
WLED_GLOBAL bool ledStatusState _INIT(false); // the current LED state
#endif

// server library objects
WLED_GLOBAL AsyncWebServer server _INIT_N(((80)));
#ifdef WLED_ENABLE_WEBSOCKETS
WLED_GLOBAL AsyncWebSocket ws _INIT_N((("/ws")));
#endif
WLED_GLOBAL AsyncClient* hueClient _INIT(NULL);
WLED_GLOBAL AsyncMqttClient* mqtt _INIT(NULL);
WLED_GLOBAL AsyncWebHandler *editHandler _INIT(nullptr);

// udp interface objects
WLED_GLOBAL WiFiUDP notifierUdp, rgbUdp, notifier2Udp;
WLED_GLOBAL WiFiUDP ntpUdp;

// Usermod manager
WLED_GLOBAL UsermodManager usermods _INIT(UsermodManager());

// global I2C SDA pin (used for usermods)
#ifndef I2CSDAPIN
WLED_GLOBAL int8_t i2c_sda  _INIT(-1);
#else
WLED_GLOBAL int8_t i2c_sda  _INIT(I2CSDAPIN);
#endif
// global I2C SCL pin (used for usermods)
#ifndef I2CSCLPIN
WLED_GLOBAL int8_t i2c_scl  _INIT(-1);
#else
WLED_GLOBAL int8_t i2c_scl  _INIT(I2CSCLPIN);
#endif

// global SPI DATA/MOSI pin (used for usermods)
#ifndef SPIMOSIPIN
WLED_GLOBAL int8_t spi_mosi  _INIT(-1);
#else
WLED_GLOBAL int8_t spi_mosi  _INIT(SPIMOSIPIN);
#endif
// global SPI DATA/MISO pin (used for usermods)
#ifndef SPIMISOPIN
WLED_GLOBAL int8_t spi_miso  _INIT(-1);
#else
WLED_GLOBAL int8_t spi_miso  _INIT(SPIMISOPIN);
#endif
// global SPI CLOCK/SCLK pin (used for usermods)
#ifndef SPISCLKPIN
WLED_GLOBAL int8_t spi_sclk  _INIT(-1);
#else
WLED_GLOBAL int8_t spi_sclk  _INIT(SPISCLKPIN);
#endif

// global ArduinoJson buffer
WLED_GLOBAL StaticJsonDocument<JSON_BUFFER_SIZE> doc;
WLED_GLOBAL volatile uint8_t jsonBufferLock _INIT(0);

// enable additional debug output
#if defined(WLED_DEBUG_HOST)
  // On the host side, use netcat to receive the log statements: nc -l 7868 -u
  // use -D WLED_DEBUG_HOST='"192.168.xxx.xxx"' or FQDN within quotes
  #define DEBUGOUT NetDebug
  WLED_GLOBAL bool netDebugEnabled _INIT(true);
  WLED_GLOBAL char netDebugPrintHost[33] _INIT(WLED_DEBUG_HOST);
  #if defined(WLED_DEBUG_NET_PORT)
  WLED_GLOBAL int netDebugPrintPort _INIT(WLED_DEBUG_PORT);
  #else
  WLED_GLOBAL int netDebugPrintPort _INIT(7868);
  #endif
#else
  #define DEBUGOUT Serial
#endif

#ifdef WLED_DEBUG
  #ifndef ESP8266
  #include <rom/rtc.h>
  #endif
  #define DEBUG_PRINT(x) DEBUGOUT.print(x)
  #define DEBUG_PRINTLN(x) DEBUGOUT.println(x)
  #define DEBUG_PRINTF(x...) DEBUGOUT.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

#ifdef WLED_DEBUG_FS
  #define DEBUGFS_PRINT(x) DEBUGOUT.print(x)
  #define DEBUGFS_PRINTLN(x) DEBUGOUT.println(x)
  #define DEBUGFS_PRINTF(x...) DEBUGOUT.printf(x)
#else
  #define DEBUGFS_PRINT(x)
  #define DEBUGFS_PRINTLN(x)
  #define DEBUGFS_PRINTF(x...)
#endif

// debug macro variable definitions
#ifdef WLED_DEBUG
  WLED_GLOBAL unsigned long debugTime _INIT(0);
  WLED_GLOBAL int lastWifiState _INIT(3);
  WLED_GLOBAL unsigned long wifiStateChangedTime _INIT(0);
  WLED_GLOBAL unsigned long loops _INIT(0);
#endif

#ifdef ARDUINO_ARCH_ESP32
  #define WLED_CONNECTED (WiFi.status() == WL_CONNECTED || ETH.localIP()[0] != 0)
#else
  #define WLED_CONNECTED (WiFi.status() == WL_CONNECTED)
#endif
#define WLED_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID, DEFAULT_CLIENT_SSID) != 0)
#define WLED_MQTT_CONNECTED (mqtt != nullptr && mqtt->connected())

#ifndef WLED_AP_SSID_UNIQUE
  #define WLED_SET_AP_SSID() do { \
    strcpy_P(apSSID, PSTR(WLED_AP_SSID)); \
  } while(0)
#else
  #define WLED_SET_AP_SSID() do { \
    strcpy_P(apSSID, PSTR(WLED_AP_SSID)); \
    snprintf_P(\
      apSSID+strlen(WLED_AP_SSID), \
      sizeof(apSSID)-strlen(WLED_AP_SSID), \
      PSTR("-%*s"), \
      6, \
      escapedMac.c_str() + 6\
    ); \
  } while(0)
#endif

//macro to convert F to const
#define SET_F(x)  (const char*)F(x)

//color mangling macros
#define RGBW32(r,g,b,w) (uint32_t((byte(w) << 24) | (byte(r) << 16) | (byte(g) << 8) | (byte(b))))
#define R(c) (byte((c) >> 16))
#define G(c) (byte((c) >> 8))
#define B(c) (byte(c))
#define W(c) (byte((c) >> 24))

class WLED {
public:
  WLED();
  static WLED& instance()
  {
    static WLED instance;
    return instance;
  }

  // boot starts here
  void setup();

  void loop();
  void reset();

  void handleConnection();
  bool initEthernet(); // result is informational
  void initAP(bool resetAP = false);
  void initConnection();
  void initInterfaces();
  void handleStatusLED();
  void enableWatchdog();
  void disableWatchdog();
};
#endif        // WLED_H
