#ifndef WLED_CONST_H
#define WLED_CONST_H

/*
 * Readability defines and their associated numerical values + compile-time constants
 */

//Defaults
#define DEFAULT_CLIENT_SSID "Your_Network"
#define DEFAULT_AP_SSID     "WLED-AP"
#define DEFAULT_AP_PASS     "wled1234"
#define DEFAULT_OTA_PASS    "wledota"

//increase if you need more
#ifndef WLED_MAX_USERMODS
  #ifdef ESP8266
    #define WLED_MAX_USERMODS 4
  #else
    #define WLED_MAX_USERMODS 6
  #endif
#endif

#ifndef WLED_MAX_BUTTONS
  #ifdef ESP8266
    #define WLED_MAX_BUTTONS 2
  #else
    #define WLED_MAX_BUTTONS 4
  #endif
#endif

//Usermod IDs
#define USERMOD_ID_RESERVED               0     //Unused. Might indicate no usermod present
#define USERMOD_ID_UNSPECIFIED            1     //Default value for a general user mod that does not specify a custom ID
#define USERMOD_ID_EXAMPLE                2     //Usermod "usermod_v2_example.h"
#define USERMOD_ID_TEMPERATURE            3     //Usermod "usermod_temperature.h"
#define USERMOD_ID_FIXNETSERVICES         4     //Usermod "usermod_Fix_unreachable_netservices.h"
#define USERMOD_ID_PIRSWITCH              5     //Usermod "usermod_PIR_sensor_switch.h"
#define USERMOD_ID_IMU                    6     //Usermod "usermod_mpu6050_imu.h"
#define USERMOD_ID_FOUR_LINE_DISP         7     //Usermod "usermod_v2_four_line_display.h
#define USERMOD_ID_ROTARY_ENC_UI          8     //Usermod "usermod_v2_rotary_encoder_ui.h"
#define USERMOD_ID_AUTO_SAVE              9     //Usermod "usermod_v2_auto_save.h"
#define USERMOD_ID_DHT                   10     //Usermod "usermod_dht.h"
#define USERMOD_ID_MODE_SORT             11     //Usermod "usermod_v2_mode_sort.h"
#define USERMOD_ID_VL53L0X               12     //Usermod "usermod_vl53l0x_gestures.h"
#define USERMOD_ID_MULTI_RELAY           13     //Usermod "usermod_multi_relay.h"
#define USERMOD_ID_ANIMATED_STAIRCASE    14     //Usermod "Animated_Staircase.h"
#define USERMOD_ID_RTC                   15     //Usermod "usermod_rtc.h"
#define USERMOD_ID_ELEKSTUBE_IPS         16     //Usermod "usermod_elekstube_ips.h"
#define USERMOD_ID_SN_PHOTORESISTOR      17     //Usermod "usermod_sn_photoresistor.h"
#define USERMOD_ID_BATTERY_STATUS_BASIC  18     //Usermod "usermod_v2_battery_status_basic.h"
#define USERMOD_ID_PWM_FAN               19     //Usermod "usermod_PWM_fan.h"
#define USERMOD_ID_BH1750                20     //Usermod "usermod_bh1750.h"
#define USERMOD_ID_SEVEN_SEGMENT_DISPLAY 21     //Usermod "usermod_v2_seven_segment_display.h"
#define USERMOD_RGB_ROTARY_ENCODER       22     //Usermod "rgb-rotary-encoder.h"
#define USERMOD_ID_QUINLED_AN_PENTA      23     //Usermod "quinled-an-penta.h"
#define USERMOD_ID_SSDR                  24     //Usermod "usermod_v2_seven_segment_display_reloaded.h"
#define USERMOD_ID_CRONIXIE              25     //Usermod "usermod_cronixie.h"
#define USERMOD_ID_WIZLIGHTS             26     //Usermod "wizlights.h"
#define USERMOD_ID_WORDCLOCK             27     //Usermod "usermod_v2_word_clock.h"
#define USERMOD_ID_MY9291                28     //Usermod "usermod_MY9291.h"
#define USERMOD_ID_SI7021_MQTT_HA        29     //Usermod "usermod_si7021_mqtt_ha.h"
#define USERMOD_ID_BME280                30     //Usermod "usermod_bme280.h
#define USERMOD_ID_SMARTNEST             31     //Usermod "usermod_smartnest.h"
#define USERMOD_ID_AUDIOREACTIVE         32     //Usermod "audioreactive.h"
#define USERMOD_ID_ANALOG_CLOCK          33     //Usermod "Analog_Clock.h"
#define USERMOD_ID_PING_PONG_CLOCK       34     //Usermod "usermod_v2_ping_pong_clock.h"
#define USERMOD_ID_ADS1115               35     //Usermod "usermod_ads1115.h"
#define USERMOD_ID_BOBLIGHT              36     //Usermod "boblight.h"
#define USERMOD_ID_SD_CARD               37     //Usermod "usermod_sd_card.h"
#define USERMOD_ID_PWM_OUTPUTS           38     //Usermod "usermod_pwm_outputs.h
#define USERMOD_ID_SHT                   39     //Usermod "usermod_sht.h

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN          0     //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN               1     //Open when no connection (either after boot or if connection is lost)
#define AP_BEHAVIOR_ALWAYS                2     //Always open
#define AP_BEHAVIOR_BUTTON_ONLY           3     //Only when button pressed for 6 sec

//Button type
#define BTN_TYPE_NONE             0
#define BTN_TYPE_RESERVED         1
#define BTN_TYPE_PUSH             2
#define BTN_TYPE_PUSH_ACT_HIGH    3
#define BTN_TYPE_SWITCH           4
#define BTN_TYPE_PIR_SENSOR       5
#define BTN_TYPE_TOUCH            6
#define BTN_TYPE_ANALOG           7
#define BTN_TYPE_ANALOG_INVERTED  8

//Ethernet board types
#define WLED_NUM_ETH_TYPES        9

#define WLED_ETH_NONE             0
#define WLED_ETH_WT32_ETH01       1
#define WLED_ETH_ESP32_POE        2
#define WLED_ETH_WESP32           3
#define WLED_ETH_QUINLED          4
#define WLED_ETH_TWILIGHTLORD     5
#define WLED_ETH_ESP32DEUX        6

// WLED Error modes
#define ERR_NONE         0  // All good :)
#define ERR_EEP_COMMIT   2  // Could not commit to EEPROM (wrong flash layout?)
#define ERR_NOBUF        3  // JSON buffer was not released in time, request cannot be handled at this time
#define ERR_JSON         9  // JSON parsing failed (input too large?)
#define ERR_FS_BEGIN    10  // Could not init filesystem (no partition?)
#define ERR_FS_QUOTA    11  // The FS is full or the maximum file size is reached
#define ERR_FS_PLOAD    12  // It was attempted to load a preset that does not exist
#define ERR_FS_IRLOAD   13  // It was attempted to load an IR JSON cmd, but the "ir.json" file does not exist
#define ERR_FS_GENERAL  19  // A general unspecified filesystem error occured
#define ERR_OVERTEMP    30  // An attached temperature sensor has measured above threshold temperature (not implemented)
#define ERR_OVERCURRENT 31  // An attached current sensor has measured a current above the threshold (not implemented)
#define ERR_UNDERVOLT   32  // An attached voltmeter has measured a voltage below the threshold (not implemented)

#define NTP_PACKET_SIZE 48

// string temp buffer (now stored in stack locally)
#ifdef ESP8266
#define SETTINGS_STACK_BUF_SIZE 2048
#else
#define SETTINGS_STACK_BUF_SIZE 3096 
#endif

#define TOUCH_THRESHOLD 32 // limit to recognize a touch, higher value means more sensitive

// Size of buffer for API JSON object (increase for more segments)
#ifdef ESP8266
  #define JSON_BUFFER_SIZE 10240
#else
  #define JSON_BUFFER_SIZE 24576
#endif

//#define MIN_HEAP_SIZE (MAX_LED_MEMORY+2048)
#define MIN_HEAP_SIZE (8192)

#define INTERFACE_UPDATE_COOLDOWN 2000 //time in ms to wait between websockets, alexa, and MQTT updates

// HW_PIN_SCL & HW_PIN_SDA are used for information in usermods settings page and usermods themselves
// which GPIO pins are actually used in a hardwarea layout (controller board)
#if defined(I2CSCLPIN) && !defined(HW_PIN_SCL)
  #define HW_PIN_SCL I2CSCLPIN
#endif
#if defined(I2CSDAPIN) && !defined(HW_PIN_SDA)
  #define HW_PIN_SDA I2CSDAPIN
#endif
// you cannot change HW I2C pins on 8266
#if defined(ESP8266) && defined(HW_PIN_SCL)
  #undef HW_PIN_SCL
#endif
#if defined(ESP8266) && defined(HW_PIN_SDA)
  #undef HW_PIN_SDA
#endif
// defaults for 1st I2C on ESP32 (Wire global)
#ifndef HW_PIN_SCL
  #define HW_PIN_SCL SCL
#endif
#ifndef HW_PIN_SDA
  #define HW_PIN_SDA SDA
#endif

// HW_PIN_SCLKSPI & HW_PIN_MOSISPI & HW_PIN_MISOSPI are used for information in usermods settings page and usermods themselves
// which GPIO pins are actually used in a hardwarea layout (controller board)
#if defined(SPISCLKPIN) && !defined(HW_PIN_CLOCKSPI)
  #define HW_PIN_CLOCKSPI SPISCLKPIN
#endif
#if defined(SPIMOSIPIN) && !defined(HW_PIN_MOSISPI)
  #define HW_PIN_MOSISPI SPIMOSIPIN
#endif
#if defined(SPIMISOPIN) && !defined(HW_PIN_MISOSPI)
  #define HW_PIN_MISOSPI SPIMISOPIN
#endif
// you cannot change HW SPI pins on 8266
#if defined(ESP8266) && defined(HW_PIN_CLOCKSPI)
  #undef HW_PIN_CLOCKSPI
#endif
#if defined(ESP8266) && defined(HW_PIN_DATASPI)
  #undef HW_PIN_DATASPI
#endif
#if defined(ESP8266) && defined(HW_PIN_MISOSPI)
  #undef HW_PIN_MISOSPI
#endif
// defaults for VSPI on ESP32 (SPI global, SPI.cpp) as HSPI is used by WLED (bus_wrapper.h)
#ifndef HW_PIN_CLOCKSPI
  #define HW_PIN_CLOCKSPI SCK
#endif
#ifndef HW_PIN_DATASPI
  #define HW_PIN_DATASPI MOSI
#endif
#ifndef HW_PIN_MISOSPI
  #define HW_PIN_MISOSPI MISO
#endif

#endif
