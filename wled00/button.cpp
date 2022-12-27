#include "wled.h"

/*
 * Physical IO
 */

#define WLED_DEBOUNCE_THRESHOLD      50 // only consider button input of at least 50ms as valid (debouncing)
#define WLED_LONG_PRESS             600 // long press if button is released after held for at least 600ms
#define WLED_DOUBLE_PRESS           350 // double press if another press within 350ms after a short press
#define WLED_LONG_REPEATED_ACTION   300 // how often a repeated action (e.g. dimming) is fired on long press on button IDs >0
#define WLED_LONG_AP               5000 // how long button 0 needs to be held to activate WLED-AP
#define WLED_LONG_FACTORY_RESET   10000 // how long button 0 needs to be held to trigger a factory reset

static const char _mqtt_topic_button[] PROGMEM = "%s/button/%d";  // optimize flash usage

void shortPressAction(uint8_t b)
{
 
}

void longPressAction(uint8_t b)
{
  
}

void doublePressAction(uint8_t b)
{
  
}

bool isButtonPressed(uint8_t i)
{
 
}

void handleSwitch(uint8_t b)
{
 
}

#define ANALOG_BTN_READ_CYCLE 250   // min time between two analog reading cycles
#define STRIP_WAIT_TIME 6           // max wait time in case of strip.isUpdating() 
#define POT_SMOOTHING 0.25f         // smoothing factor for raw potentiometer readings
#define POT_SENSITIVITY 4           // changes below this amount are noise (POT scratching, or ADC noise)

void handleAnalog(uint8_t b)
{
 
}

void handleButton()
{
  
}

// If enabled, RMT idle level is set to HIGH when off
// to prevent leakage current when using an N-channel MOSFET to toggle LED power
#ifdef ESP32_DATA_IDLE_HIGH
void esp32RMTInvertIdle()
{
  bool idle_out;
  for (uint8_t u = 0; u < busses.getNumBusses(); u++)
  {
    if (u > 7) return; // only 8 RMT channels, TODO: ESP32 variants have less RMT channels
    Bus *bus = busses.getBus(u);
    if (!bus || bus->getLength()==0 || !IS_DIGITAL(bus->getType()) || IS_2PIN(bus->getType())) continue;
    //assumes that bus number to rmt channel mapping stays 1:1
    rmt_channel_t ch = static_cast<rmt_channel_t>(u);
    rmt_idle_level_t lvl;
    rmt_get_idle_level(ch, &idle_out, &lvl);
    if (lvl == RMT_IDLE_LEVEL_HIGH) lvl = RMT_IDLE_LEVEL_LOW;
    else if (lvl == RMT_IDLE_LEVEL_LOW) lvl = RMT_IDLE_LEVEL_HIGH;
    else continue;
    rmt_set_idle_level(ch, idle_out, lvl);
  }
}
#endif

void handleIO()
{
 
}