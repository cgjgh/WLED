#include "inttypes.h"
#include "Arduino.h"

#ifndef WLED_CONST_LEDCLOCK_H
#define WLED_CONST_LEDCLOCK_H

#define MDNS_PREFIX "ledclock"
#define MQTT_DEVICE_TOPIC "ledclock"
#define MQTT_CLIENT_ID "LEDCLOCK"

#define BUZZER_PIN 12
#define ADC_MAX_VALUE 4095
#define ADC_MAX_VOLTAGE 3.3
#define BRIGHTNESS_SAMPLES 1024
#define BRIGHTNESS_THRESHOLD 5
#define BRIGHTNESS_PIN 34

#define STOPWATCH_MAX_LAP_TIMES 100

#define BEEP_SILENT 255

// How to configure the display?
//
// Think of your clock display as 2D matrix of pixels rather than a 7 segment display.
//
// Some pixels of this 2D matrix are missing, others are present. To get the layout
// where the pixels are missing or present, first decide on how many LEDs you want per segment.
//
// For example,  if you have only one LED per segment, the layout is the following:
//
//  -#--#---#--#-
//  #-##-###-##-#
//  -#--#---#--#-
//  #-##-###-##-#
//  -#--#---#--#-
//
// Where the symbol '-' represents a missing pixel and symbol '#' represents a pixel that presents.
// Note the two 'separator' pixels in the middle 'horizontal' column.
//
// If you have 2 LEDs per segment,  the layout is the following:
//
// -##--##---##--##-
// #--##--#-#--##--#
// #--##--###--##--#
// -##--##---##--##-
// #--##--###--##--#
// #--##--#-#--##--#
// -##--##---##--##-
//
// Next you need to replace all the '-' symbols with -1 and all the '#' symbols with LED indices. Which index
// you write in place of a particular '#' symbol depends on the physical layout of your LED strip.
//
// In the current configuration below, there are 10 LEDs per segment. 
// The LED strip originates at the rightmost digit (Digit 4) and winds leftwards up to Digit 1. 
// Each digit's segments are wired continuously in the following snake pattern:
//    - Segment 1: Top Left (goes bottom-to-top)
//    - Segment 2: Top (goes left-to-right)
//    - Segment 3: Top Right (goes top-to-bottom)
//    - Segment 4: Bottom Right (goes top-to-bottom)
//    - Segment 5: Bottom (goes right-to-left)
//    - Segment 6: Bottom Left (goes bottom-to-top)
//    - Segment 7: Middle (goes left-to-right)
// The separator (colon) consists of 8 LEDs placed between the hour and minute blocks.
//
// For this 10-LED config, the total dimensions result in a 49x23 grid of numbers. Empty padding spaces are labeled -1.

#define LC_LEDS_PER_SEGM 10 // LEDs per segment

#define LC_LEDMAP \
 -1,258,259,260,261,262,263,264,265,266,267, -1, -1,188,189,190,191,192,193,194,195,196,197, -1, -1, -1,110,111,112,113,114,115,116,117,118,119, -1, -1, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, -1, \
257, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,268,187, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,198, -1,109, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,120, 39, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 50, \
256, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,269,186, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,199, -1,108, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,121, 38, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 51, \
255, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,270,185, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,200, -1,107, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,122, 37, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 52, \
254, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,271,184, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,201, -1,106, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,123, 36, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 53, \
253, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,272,183, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,202, -1,105, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,124, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 54, \
252, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,273,182, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,203,147,104, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,125, 34, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 55, \
251, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,274,181, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,204,146,103, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,126, 33, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 56, \
250, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,275,180, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,205,145,102, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,127, 32, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 57, \
249, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,276,179, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,206,144,101, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,128, 31, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 58, \
248, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,277,178, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,207, -1,100, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,129, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 59, \
 -1,287,286,285,284,283,282,281,280,279,278, -1, -1,217,216,215,214,213,212,211,210,209,208, -1, -1, -1,139,138,137,136,135,134,133,132,131,130, -1, -1, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, -1, \
247, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,218,177, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,148, -1, 99, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 70, 29, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, \
246, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,219,176, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,149,143, 98, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 71, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, \
245, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,220,175, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,150,142, 97, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 72, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, \
244, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,221,174, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,151,141, 96, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 73, 26, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  3, \
243, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,222,173, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,152,140, 95, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 74, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4, \
242, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,223,172, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,153, -1, 94, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 75, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5, \
241, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,224,171, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,154, -1, 93, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 76, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, \
240, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,225,170, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,155, -1, 92, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 77, 22, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  7, \
239, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,226,169, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,156, -1, 91, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 78, 21, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  8, \
238, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,227,168, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,157, -1, 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 79, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  9, \
 -1,237,236,235,234,233,232,231,230,229,228, -1, -1,167,166,165,164,163,162,161,160,159,158, -1, -1, -1, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, -1, -1, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, -1


// Next configure the separator LEDs by first defining how many of them you have:
#define LC_SEP_LEDS 8

// Then write their row indices separated by commas below (notice the separator LEDs #28 and #29 having row indices 2 and 4 in the above LED map):
#define LC_SEP_LED_ROWS 6, 7, 8, 9, 13, 14, 15, 16

// Finally, don't forget to change the total number of LEDs (DEFAULT_LED_COUNT) in `platformio_override.ini`,
// set it to the total size of the LED matrix you have. This number is calculated below and a compiler warning
// is emitted if it does not match with DEFAULT_LED_COUNT.

#define LC_COLS     (((LC_LEDS_PER_SEGM + 2) * 4) + 1)
#define LC_ROWS      ((LC_LEDS_PER_SEGM * 2) + 3)
#define LC_TOTAL_LEDS (LC_COLS * LC_ROWS) // DEFAULT_LED_COUNT should be set to this value

#if !defined(DEFAULT_LED_COUNT) || DEFAULT_LED_COUNT != LC_TOTAL_LEDS
#define LC_XSTR(x) LC_STR(x)
#define LC_STR(x) #x
#pragma message "Macro DEFAULT_LED_COUNT is not defined or is not equal to the calculated number of total LEDs: " LC_XSTR(LC_TOTAL_LEDS)
#endif

// Your display still does not work properly? Verify all the steps above, and if everything looks right, follow these steps:
//   1. navigate your browser http://wled-ip/edit
//   2. right click on /ledmap.json
//   3. choose 'Delete'
//   4. reboot WLED
// The file ledmap.json should now be regenerated with the correct settings and your display should now work properly.


#define LC_PHYSICAL_LEDS (LC_SEP_LEDS + 4 * 7 * LC_LEDS_PER_SEGM)

#define LC_R(c) (byte((c) >> 16))
#define LC_G(c) (byte((c) >> 8))
#define LC_B(c) (byte(c))
#define LC_W(c) (byte((c) >> 24))

class LedClockSettingsKeys {
public:
    static const char *root;

    class Brightness {
    public:
        static const char *autom, *min, *max;
    };

    class Display {
    public:
        static const char *separatorMode, *hideZero;
    };

    class Beeps {
    public:
        static const char *mute, *startup, *wifi;

        class Clock {
        public:
            static const char *minute, *hour;
        };

        class Timer {
        public:
            static const char *set, *start, *pause, *resume, *reset, *increase, *hour, *minute, *second, *timeout;
        };

        class Stopwatch {
        public:
            static const char *start, *pause, *resume, *reset, *second, *minute, *hour, *lapTime;
        };
    };
};

class LedClockStateKeys {
public:
    static const char *root, *command, *mode, *beep, *blendingMode, *canvasColor;

    class Timer {
    public:
        static const char *root, *running, *paused, *left, *value;
    };

    class Stopwatch{
    public:
        static const char *root, *running, *paused, *elapsed, *lapTimes, *lapTimeNr, *lastLapTime;
    };
};

class LedClockSettings {

public:
    enum SeparatorMode {
        ON, OFF, BLINK
    };

    virtual ~LedClockSettings() {}
    bool autoBrightness = true;
    uint8_t minBrightness = 50; // must NOT be lower than 1
    uint8_t maxBrightness = 255;
    SeparatorMode separatorMode = SeparatorMode::BLINK;
    bool hideZero = true;

    bool muteBeeps = false;

    uint8_t beepStartup, beepWiFi;
    uint8_t clockBeepMinute, clockBeepHour;
    uint8_t timerBeepSet, timerBeepStart, timerBeepPause, timerBeepResume, timerBeepReset, timerBeepIncrease, timerBeepHour, timerBeepMinute, timerBeepSecond, timerBeepTimeout;
    uint8_t stopwatchBeepStart, stopwatchBeepPause, stopwatchBeepResume, stopwatchBeepReset, stopwatchBeepSecond, stopwatchBeepMinute, stopwatchBeepHour, stopwatchBeepLapTime;

    virtual void applySettings() = 0;

    static uint8_t constrainBeep(uint8_t beep);
};

const char JSON_ledclock_beeps[] PROGMEM = R"=====([
"1x 330Hz (short)",
"2x 330Hz (short)",
"3x 330Hz (short)",
"1x 440Hz (short)",
"2x 440Hz (short)",
"3x 440Hz (short)",
"1x 880Hz (short)",
"2x 880Hz (short)",
"3x 880Hz (short)",
"1x 330Hz (medium)",
"2x 330Hz (medium)",
"3x 330Hz (medium)",
"1x 440Hz (medium)",
"2x 440Hz (medium)",
"3x 440Hz (medium)",
"1x 880Hz (medium)",
"2x 880Hz (medium)",
"3x 880Hz (medium)",
"1x 330Hz (long)",
"2x 330Hz (long)",
"3x 330Hz (long)",
"1x 440Hz (long)",
"2x 440Hz (long)",
"3x 440Hz (long)",
"1x 880Hz (long)",
"2x 880Hz (long)",
"3x 880Hz (long)",
"440/880Hz (short)",
"880/440Hz (short)",
"440/880Hz (medium)",
"880/440Hz (medium)",
"440/880Hz (long)",
"880/440Hz (long)",
"Turn Up",
"Turn Down",
"Flip Up",
"Flip Down",
"Tadaaa"
])=====";

// custom effects

#define FX_MODE_LC_2SOFIX     187
#define FX_MODE_LC_VORTEX     188
#define FX_MODE_LC_CONCENTRIC 189

// forward declarations

void ledClockTimeUpdated();

#endif