#pragma once

#include "wled.h"
#include <NeoPixelBus.h>
#include <RunningMedian.h>
#include <ModbusIP_ESP8266.h>

#pragma region leds

#define COLOR_SATURATION 255

const uint16_t PixelCount = 24;
const uint8_t PixelPin = 15;
uint8_t colorSaturation = COLOR_SATURATION;

// LED Start Index
byte autoLedStart = 0;
byte stallLedStart = 6;
byte cowFWDSafetyLedStart = 12;
byte spareLEDStart = 18;

enum Light
{
  autoStop = 0,
  stall = 1,
  cowFWD = 2,
  spare = 3,
};

// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor orange(colorSaturation, 80, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);
RgbColor purple(colorSaturation / 2, 0, colorSaturation / 2);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

// LED states
RgbColor autoLedState = black;
RgbColor stallLedState = black;
RgbColor cowFWDSafetyLedState = black;
byte statusLedState = 0;
RgbColor spareLedState = black;

#pragma endregion leds

#pragma region Modbus Callbacks

bool stallRegChanged = 0;
bool toggleRegChanged = 0;
uint16_t cbStallChange(TRegister *reg, uint16_t val)
{
  // Check if reg state is going to be changed
  if (reg->value != val)
  {
    DEBUG_PRINT(PSTR("Set reg: "));
    Serial.println(val);
    stallRegChanged = 1;
    if (val > (reg->value + 1))
    {
      DEBUG_PRINTLN(PSTR("Stall skip"));
      DEBUG_PRINTLN(PSTR("New:"));
      DEBUG_PRINT(val);
      DEBUG_PRINTLN(PSTR("Old:"));
      DEBUG_PRINT(reg->value);
    }
  }
  // stallRegChanged = 1;
  return val;
}

uint16_t cbToggleChange(TRegister *reg, uint16_t val)
{
  // Check if reg state is going to be changed
  if (reg->value != val)
  {
    DEBUG_PRINT(PSTR("Set reg: "));
    Serial.println(val);
    toggleRegChanged = 1;
    if (val > (reg->value + 1))
    {
      DEBUG_PRINTLN(PSTR("Toggle skip"));
      DEBUG_PRINTLN(PSTR("New:"));
      DEBUG_PRINT(val);
      DEBUG_PRINTLN(PSTR("Old:"));
      DEBUG_PRINT(reg->value);
    }
  }
  return val;
}

#pragma endregion Modbus Callbacks

class ParlorControl : public Usermod
{

private:
  // Private class members. You can declare variables and functions only accessible to your usermod here

#pragma region states
  enum AutoMode
  {
    fullAuto = 1,
    groupSplit = 2,
    prep = 3,
    washCIP = 4,
    off = 5,
  };

  enum MilkerWashMode
  {
    valveOff = 0,
    valveOn = 1,
    onFWD = 2,
    onWashCIP = 3,
  };

  enum ParlorWashMode
  {
    pValveOff = 0,
    pValveOn = 1,
    pOnFWD = 2,
    onFWDNoPrep = 3,
  };

  enum WaterChaserMode
  {
    chaserOff = 0,
    chaserOn = 1,
    onDetect = 2,

  };

  enum HornMode
  {
    hornOff = 0,
    hornOn = 1,
    stopFailure = 2,
    chase = 3,
  };

  AutoMode currentMode = off;
  MilkerWashMode milkerWashMode = onWashCIP;
  WaterChaserMode waterChaserMode = onDetect;
  HornMode hornMode = stopFailure;
  ParlorWashMode parlorWashMode = onFWDNoPrep;

#pragma endregion states

#pragma region variables

  bool autoStopEnabled = 1;
  bool splitterDisabled = 1;
  bool enableStatusLED = 1;

  // LED Defaults
  const int statusLedOnTime = 1000;
  const int stallLedOnTime = 1000;
  const int rebootLightOnTime = 4000;

  // read intervals
  const byte inputReadInterval = 20;
  const byte modbusReadInterval = 20;

  // delay defaults
  const int minDelayBetweenStalls = 750;
  const int parlorStopFailureDelay = 750;
  const int exitFWDSafetyDelay = 750;
  const ulong checkCIPInterval = 1200000;
  bool CIPUpdated = false;

  // stall vars
  const byte initialStall = 27;
  byte currentStall = initialStall;
  byte CIPStall = 0;
  byte stallsBetweenGroup = 4;

  float speedSetpoint = 13.0;
  float errorMargin = 1.2;
  bool stallChangeStatusLED = 0;
  bool speedLEDEnabled = 1;

  // max/min sampled speed in sec per stall
  const float maxSecPerStall = 25.0;
  const float minSecPerStall = 4.0;

  // millisecond offset to center stall
  const int maxOffsetMS = 5000;
  const int minOffsetMS = 2000;

  // percentage factor for position of stall at stall change signal (1 = perfectly aligned, .5 = in between stalls)
  float stallPosAtStallChange = .4;

  // input pin assignments
  const byte input1 = 19;
  const byte input2 = 18;
  const byte input3 = 5;
  const byte input4 = 17;
  const byte input5 = 16;
  const byte input6 = 4;

  // relay pin assignments
  const byte relay1 = 12;
  const byte relay2 = 14;
  const byte relay3 = 33;
  const byte relay4 = 27;
  const byte relay5 = 26;

  // input pin assignments
  const byte stallSensorPin = input1;
  const byte splitterSwitchPin = input2;
  const byte cowFWDsafetySensorPin = input3;
  const byte cowFWDsafetySensorPin2 = input4;
  const byte parlorFWDPin = input5;
  const byte cowFWDsafetySwitchPin = input6;

  // relay pin assignments
  const byte parlorPauseRelayPin = relay1;
  const byte hornRelayPin = relay2;
  const byte milkerWashRelayPin = relay3;
  const byte waterRelayPin = relay4;
  const byte parlorWashRelayPin = relay5;

  // relay states
  bool parlorStopRelayState = 0;
  bool hornRelayState = 0;
  bool waterRelayState = 0;
  bool milkerWashRelayState = 0;
  bool parlorWashRelayState = 0;

  // millis() time of last...
  unsigned long lastStallChange = 0;
  unsigned long lastLEDBlink = 0;
  unsigned long lastStatusLEDBlink = 0;
  unsigned long lastStallLEDBlink = 0;
  unsigned long lastInputRead = 0;
  unsigned long lastSpeedLEDSet = 0;
  unsigned long lastCowFWDSafetySwitchNotTriggered = 0;
  unsigned long lastParlorStopRelayTriggered = 0;
  unsigned long lastFWDSafetyNotTrig = 0;
  unsigned long lastFWDSafetyCheck = 0;
  unsigned long lastModbusRead = 0;
  unsigned long lastModbusTask = 0;

  // parlor vars
  bool stallChange = 0;
  bool IDChange = 0;
  float parlorSpeed = 0;
  bool stallEmpty = 0;
  bool cowFWDSafety = 0;
  bool cowFWDSafety2 = 0;
  bool parlorFWD = 0;
  bool cowFWDSafetySwitch = 0;
  bool parlorStopFailureDetected = 0;
  bool gotCIP = 0;
  int conseqSkippedStalls = 0;
  byte groupsDone = 0;
  bool newGroup = 0;
  bool completed = 1;
  bool resetOnNextSwitchRelease = 0;
  bool showRebootStatus = 1;

  // parlor stat vars
  int totalFullStalls = 0;
  int totalEmptyStallStops = 0;
  int totalFwdSftStops = 0;
  int totalInGroup[10];
  float avgTimePerStall;
  time_t startTime;
  time_t endTime;
  bool statsReset = 1;
  int totalStopFailures = 0;
  int totalSafetySwitchTriggers = 0;

  // Modbus change tracking
  uint16_t stallCounter = 0;
  uint16_t toggleCounter = 0;
  uint16_t ropeSwitchCounter = 0;

  // Modbus register addresses
  uint8_t stallReg = 100;
  uint8_t toggleReg = 101;
  uint8_t idReg = 102;
  uint8_t ropeReg = 103;

  // misc
  bool speedMeasured = 0;
  ulong timeToStallCenterPos = 0;

#pragma endregion variables

  enum Relays
  {
    parlor = 1,
    horn = 2,
    water = 3,
    milkerWash = 4,
    parlorWash = 5,
  };

  // speed samplings
  RunningMedian speedSamples = RunningMedian(5);

  // Modbus TCP server
  ModbusIP mb;

  bool enabled = true;
  bool initDone = false;

  // string that are used multiple time (this will save some flash memory)
  static const char _name[];
  static const char _enabled[];

  // any private methods should go here (non-inline method should be defined out of class)
  void publishMqtt(const char *state, const char *topic, bool retain = false); // example for publishing MQTT message
  void setLEDRange(Light light, RgbColor color);
  void setLightColor(Light light, RgbColor color);
  void hsvToRgb(byte h, byte s, byte v, byte &r, byte &g, byte &b);
  RgbColor generateSpeedColor(float inputValue);
  void setRelay(Relays relay, bool state);
  float round_to_dp(float in_value, int decimal_place); // round float to x decimal places
  void getMedianSpeed();                                // calculate the median speed from millis()
  void increment_stall();
  void stall_Change();
  void resetStats();
  void setMode(AutoMode mode);
  void setGroup(byte group);

public:
  /**
   * Enable/Disable the usermod
   */
  inline void enable(bool enable) { enabled = enable; }

  /**
   * Get usermod enabled/disabled state
   */
  inline bool isEnabled() { return enabled; }

  // externally accessible methods
  bool onMqttMessage(char *topic, char *payload);
  void onMqttConnect(bool sessionPresent);

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * readFromConfig() is called prior to setup()
   * You can use it to initialize variables, sensors or similar.
   */
  void setup()
  {
    // initialize inputs
    pinMode(stallSensorPin, INPUT);
    pinMode(splitterSwitchPin, INPUT);
    pinMode(cowFWDsafetySensorPin, INPUT);
    pinMode(cowFWDsafetySensorPin2, INPUT);
    pinMode(parlorFWDPin, INPUT);
    pinMode(cowFWDsafetySwitchPin, INPUT);

    // initialize Light
    strip.Begin();

    // initialize relay pin outputs
    pinMode(parlorPauseRelayPin, OUTPUT);
    pinMode(hornRelayPin, OUTPUT);
    pinMode(waterRelayPin, OUTPUT);
    pinMode(milkerWashRelayPin, OUTPUT);
    pinMode(parlorWashRelayPin, OUTPUT);

    // initial setting
    digitalWrite(parlorPauseRelayPin, LOW);
    digitalWrite(hornRelayPin, LOW);
    digitalWrite(waterRelayPin, LOW);
    digitalWrite(milkerWashRelayPin, LOW);
    digitalWrite(parlorWashRelayPin, LOW);
    digitalWrite(2, LOW);

    initDone = true;

    // reboot status indicator
    setLEDRange(autoStop, blue);
    setLEDRange(stall, blue);
    setLEDRange(cowFWD, blue);
    setLEDRange(spare, blue);
  }

  /*
   * connected() is called every time the WiFi is (re)connected
   * Use it to initialize network interfaces
   */
  void connected()
  {
    IPAddress myIP = WiFi.softAPIP();
    DEBUG_PRINT(PSTR("AP IP address: "));
    Serial.println(myIP);
    mb.server(); // Start Modbus IP
    mb.addHreg(stallReg, 0, 1);
    mb.addHreg(toggleReg, 0, 1);
    mb.addHreg(idReg, 0, 1);
    mb.addHreg(ropeReg, 0, 1);
    mb.onSetHreg(stallReg, cbStallChange);   // Add callback on Reg Stall value set
    mb.onSetHreg(toggleReg, cbToggleChange); // Add callback on Reg Stall value set

    DEBUG_PRINTLN(PSTR("--------------------------------------------------------------------------------"));
    DEBUG_PRINTLN(PSTR("--------------------------------------------------------------------------------"));
    DEBUG_PRINTLN(PSTR("--------------------------------------------------------------------------------"));
    DEBUG_PRINTLN(PSTR("--------------------------------------------------------------------------------"));
    DEBUG_PRINTLN(PSTR("Rebooted/connected"));
  }

  void loop()
  {
    // reboot status indicator
    static const unsigned long lastReboot = millis();

    // if usermod is disabled or called during strip updating just exit
    if (!enabled)
      return;

    // Modbus TCP

    // Modbus "magic"
    if (millis() - lastModbusTask > 10)
    {
      mb.task();
    }

    // check stall status
    if (stallRegChanged)
    {
      stallRegChanged = 0;
      stall_Change();
    }

    // check toggle status
    if (toggleRegChanged)
    {
      toggleRegChanged = 0;
      if (autoStopEnabled)
      {
        autoStopEnabled = 0;
      }
      else
      {
        autoStopEnabled = 1;
      }
    }

    // set reboot status light
    if (millis() - lastReboot > rebootLightOnTime && showRebootStatus)
    {
      showRebootStatus = 0;
      setLEDRange(autoStop, black);
      setLEDRange(stall, black);
      setLEDRange(cowFWD, black);
      setLEDRange(spare, black);
    }

    if (showRebootStatus)
    {
      // strip.Show();
    }

    // set relays
    if (waterChaserMode == chaserOff)
    {
      setRelay(water, 0);
    }
    else if (waterChaserMode == chaserOn)
    {
      setRelay(water, 1);
    }

    if (milkerWashMode == valveOff)
    {
      setRelay(milkerWash, 0);
    }
    else if (milkerWashMode == valveOn)
    {
      setRelay(milkerWash, 1);
    }

    if (parlorWashMode == pValveOff)
    {
      setRelay(parlorWash, 0);
    }
    else if (parlorWashMode == pValveOn)
    {
      setRelay(parlorWash, 1);
    }

    if (hornMode == hornOff)
    {
      setRelay(horn, 0);
    }
    else if (hornMode == hornOn)
    {
      setRelay(horn, 1);
    }

    // check inputs
    if (millis() - lastInputRead > inputReadInterval)
    {
      // check if bypass switch is on
      splitterDisabled = digitalRead(splitterSwitchPin);

      // check if stall status has changed
      if (autoStopEnabled)
      {
        if (splitterDisabled)
        {
          setMode(fullAuto);
          setLightColor(autoStop, green);
          if (newGroup)
          {
            resetOnNextSwitchRelease = 1;
          }
          // else
          // {
          //   resetOnNextSwitchRelease = 0;
          // }
        }

        else
        {
          if (groupsDone > 3 || completed == 1)
          {
            setMode(washCIP);
            setLightColor(autoStop, blue);

            // check if parlor stop relay is on
            if (parlorStopRelayState)
            {
              setLightColor(stall, orange);
              setRelay(parlor, 0);
            }
          }

          else if (currentMode == prep)
          {
            setLightColor(autoStop, purple);

            // check if parlor stop relay is on
            if (resetOnNextSwitchRelease)
            {
              newGroup = 0;
              resetOnNextSwitchRelease = 0;
            }
            if (parlorStopRelayState && (newGroup == 0))
            {
              setLightColor(stall, orange);
              setRelay(parlor, 0);
            }
          }

          else
          {
            setMode(groupSplit);
            setLightColor(autoStop, orange);
            // check if parlor stop relay is on

            if (resetOnNextSwitchRelease)
            {
              newGroup = 0;
              resetOnNextSwitchRelease = 0;
            }
            if (parlorStopRelayState && (newGroup == 0))
            {
              setLightColor(stall, orange);
              setRelay(parlor, 0);
            }
          }
        }

        stallEmpty = digitalRead(stallSensorPin);
        if ((!stallEmpty) && !(currentMode == groupSplit && (newGroup == 1)) && !(currentMode == prep && (newGroup == 1)))
        {
          if (!(stallLedState == black))
          {
            setLightColor(stall, green);
            if (currentMode == fullAuto)
            {
              statsReset = 0;
              totalFullStalls += 1;
              totalInGroup[groupsDone] += 1;
            }
          }

          // check if parlor stop relay is on
          if (parlorStopRelayState)
          {
            setRelay(parlor, 0);
          }
        }
      }
      else
      {
        completed = 1;
        setGroup(0);
        setMode(off);

        setLightColor(autoStop, red);

        // check if parlor stop relay is on

        if (!stallEmpty)
        {
          if (stallLedState == red)
          {
            setLightColor(stall, green);
          }

          if (parlorStopRelayState)
          {
            setRelay(parlor, 0);
          }
        }

        else
        {
          if (stallLedState == red)
          {
            setLightColor(stall, orange);
          }

          if (parlorStopRelayState)
          {
            setRelay(parlor, 0);
          }
        }
      }

      // check if cow forward safety sensor is triggered
      cowFWDSafety = digitalRead(cowFWDsafetySensorPin);
      lastFWDSafetyCheck = millis();

      if (!cowFWDSafety) // && millis() - lastFWDSafetyNotTrig > exitFWDSafetyDelay)
      {
        setLightColor(cowFWD, red);
        if (waterChaserMode == onDetect)
        {
          setRelay(water, (1));
        }
        totalFwdSftStops += 1;
      }
      else
      {
        lastFWDSafetyNotTrig = millis();
        setLightColor(cowFWD, black);
        if (waterChaserMode == onDetect)
        {
          setRelay(water, (0));
        }
      }

      // check if cow forward safety switch is triggered
      bool tempCowFWDSafetySwitch = digitalRead(cowFWDsafetySwitchPin);
      if (tempCowFWDSafetySwitch)
      {
        setLightColor(cowFWD, red);
        if (!cowFWDSafetySwitch)
        {
          cowFWDSafetySwitch = 1;
          totalSafetySwitchTriggers += 1;
        }
      }
      else if (cowFWDSafety && !tempCowFWDSafetySwitch)
      {
        cowFWDSafetySwitch = 0;
        setLightColor(cowFWD, black);
        lastCowFWDSafetySwitchNotTriggered = millis();
      }
      else if (!tempCowFWDSafetySwitch)
      {
        lastCowFWDSafetySwitchNotTriggered = millis();
      }

      // check if parlor is in forward motion
      parlorFWD = digitalRead(parlorFWDPin);
      if (parlorFWD)
      {
      }
      else
      {
      }

      // set milker wash mode on FWD
      if (milkerWashMode == onWashCIP)
      {

        if (currentMode == washCIP && parlorFWD)
        {
          setRelay(milkerWash, 1);
        }
        else
        {
          setRelay(milkerWash, 0);
        }
      }

      else if (milkerWashMode == onFWD)
      {

        if (parlorFWD)
        {
          setRelay(milkerWash, 1);
        }
        else
        {
          setRelay(milkerWash, 0);
        }
      }

      // set parlor wash mode on FWD
      if (parlorWashMode == onFWDNoPrep)
      {

        if (currentMode != prep && parlorFWD)
        {
          setRelay(parlorWash, 1);
        }
        else
        {
          setRelay(parlorWash, 0);
        }
      }

      else if (parlorWashMode == pOnFWD)
      {

        if (parlorFWD)
        {
          setRelay(parlorWash, 1);
        }
        else
        {
          setRelay(parlorWash, 0);
        }
      }
      // if (hornMode == stopFailure)
      // {
      // parlor stop failure detection from safety switch
      if (millis() - lastCowFWDSafetySwitchNotTriggered > parlorStopFailureDelay && parlorFWD && tempCowFWDSafetySwitch)
      {
        if (!parlorStopFailureDetected)
        {
          parlorStopFailureDetected = 1;
          totalStopFailures += 1;
        }

        setLightColor(cowFWD, red);
        setRelay(horn, 1);
      }
      //}

      // // parlor stop failure detection from stall skip safety
      // else if (millis() - lastParlorStopRelayTriggered > parlorStopFailureDelay && parlorFWD && parlorStopRelayState)
      // {
      //   if (!parlorStopFailureDetected)
      //   {
      //     parlorStopFailureDetected = 1;
      //     totalStopFailures += 1;
      //   }
      //   setLightColor(cowFWD, red);
      //   setRelay(horn, 1);
      // }
      else
      {
        parlorStopFailureDetected = 0;
        if (hornMode == stopFailure)
        {
          setRelay(horn, 0);
        }
      }

      lastInputRead = millis();
    }

    if (stallChange)
    {
      if (!speedMeasured)
      {
        getMedianSpeed();
      }

      if (millis() > (lastStallChange + timeToStallCenterPos))
      {

        if (currentMode == washCIP && statsReset == 1 && CIPStall > 0)
        {
          setMode(prep);
          completed = 0;
        }

        if (currentMode == prep && groupsDone == 0 && currentStall == CIPStall)
        {
          newGroup = 1;
          setLightColor(stall, red);
          setRelay(parlor, 1);
        }

        // stallEmpty = digitalRead(stallSensorPin);
        if (!stallEmpty)
        {
          setLightColor(stall, green);

          if (autoStopEnabled && splitterDisabled)
          {
            setRelay(parlor, 0);
            completed = 0;
            conseqSkippedStalls = 0;
            newGroup = 0;
            statsReset = 0;
            totalFullStalls += 1;
            totalInGroup[groupsDone] += 1;
          }
        }

        // stall is empty on stall change
        else
        {

          if (autoStopEnabled)
          {
            if (splitterDisabled)
            {
              setLightColor(stall, red);
              setRelay(parlor, 1);
              totalEmptyStallStops += 1;
            }

            else
            {
              if (currentMode == prep)
              {
              }

              else if (groupsDone < 4 && completed != 1)
              {
                setMode(groupSplit);
                conseqSkippedStalls += 1;

                if (conseqSkippedStalls == (stallsBetweenGroup))
                {
                  setGroup(groupsDone + 1);
                }

                if (conseqSkippedStalls == (stallsBetweenGroup + 1))
                {
                  if (groupsDone < 4)
                  {
                    newGroup = 1;
                    setLightColor(stall, red);
                    setRelay(parlor, 1);
                  }

                  else
                  {
                    newGroup = 0;
                    completed = 1;
                    setGroup(0);
                    setMode(washCIP);
                    setLightColor(stall, orange);
                    setRelay(parlor, 0);
                  }
                }

                else
                {
                  setLightColor(stall, orange);
                  setRelay(parlor, 0);
                }
              }
              else
              {

                setLightColor(stall, orange);
                setRelay(parlor, 0);
              }
            }
          }
          else
          {
            setLightColor(stall, orange);
            setRelay(parlor, 0);
          }
        }
        DEBUG_PRINT(F("Current Stall: "));
        DEBUG_PRINTLN(currentStall);
        stallChange = 0;
        char stallStr[3];
        snprintf_P(stallStr, sizeof(stallStr), PSTR("%d"), currentStall);
        publishMqtt(stallStr, PSTR("/stat/stall"), true);
      }
    }

    if (stallChangeStatusLED)
    {

      if (millis() - lastStatusLEDBlink > 1000 && spareLedState == blue)
      {
        digitalWrite(2, 0);
        setLightColor(spare, black);
      }

      else
      {
      }
    }
    else if (speedLEDEnabled)
    {
      if (millis() - lastSpeedLEDSet > 20000 && spareLedState != black)
      {
        setLightColor(spare, black);
      }
    }

    // if (!stallAlwaysOn)
    if (true)
    {

      if (stallLedState == green || stallLedState == orange)
      {
        if (millis() - lastStallLEDBlink > 4000)
        {
          setLightColor(stall, black);
        }
      }
    }

    byte hr = hour(localTime);

    if (hr == 12 || hr == 0)
    {
      resetStats();
      currentStall = initialStall;
    }
  }

#pragma region Config
  /*
   * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
   * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
   * Below it is shown how this could be used for e.g. a light sensor
   */
  void addToJsonInfo(JsonObject &root)
  {
    // if "u" object does not exist yet wee need to create it
    // JsonObject user = root["u"];
    // if (user.isNull())
    //   user = root.createNestedObject("u");

    // this code adds "u":{"ExampleUsermod":[20," lux"]} to the info object
    // int reading = 20;
    // JsonArray lightArr = user.createNestedArray(FPSTR(_name))); //name
    // lightArr.add(reading); //value
    // lightArr.add(F(" lux")); //unit

    // if you are implementing a sensor usermod, you may publish sensor data
    // JsonObject sensor = root[F("sensor")];
    // if (sensor.isNull()) sensor = root.createNestedObject(F("sensor"));
    // temp = sensor.createNestedArray(F("light"));
    // temp.add(reading);
    // temp.add(F("lux"));
  }

  /*
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void addToJsonState(JsonObject &root)
  {
    if (!initDone || !enabled)
      return; // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_name)];
    if (usermod.isNull())
      usermod = root.createNestedObject(FPSTR(_name));

    usermod["autoStopEnabled"] = autoStopEnabled;
    usermod["speedSetpoint"] = speedSetpoint;
    usermod["currentStall"] = currentStall;
    usermod["milkerWashMode"] = milkerWashMode;
    usermod["waterChaserMode"] = waterChaserMode;
    usermod["parlorWashMode"] = parlorWashMode;
    usermod["CIPStall"] = CIPStall;
    usermod["groupsDone"] = groupsDone;
    usermod["hornMode"] = hornMode;
    usermod["currentMode"] = currentMode;
  }

  /*
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void readFromJsonState(JsonObject &root)
  {
    if (!initDone)
      return; // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_name)];
    if (!usermod.isNull())
    {
      // expect JSON usermod data in usermod name object: {"ExampleUsermod:{"user0":10}"}
      userVar0 = usermod["user0"] | userVar0; // if "user0" key exists in JSON, update, else keep old value
    }
    // you can as well check WLED state JSON keys
    // if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
  }

  /*
   * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
   * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
   * If you want to force saving the current state, use serializeConfig() in your loop().
   *
   * CAUTION: serializeConfig() will initiate a filesystem write operation.
   * It might cause the LEDs to stutter and will cause flash wear if called too often.
   * Use it sparingly and always in the loop, never in network callbacks!
   *
   * addToConfig() will make your settings editable through the Usermod Settings page automatically.
   *
   * Usermod Settings Overview:
   * - Numeric values are treated as floats in the browser.
   *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
   *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
   *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
   *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
   *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
   *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
   *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
   *     used in the Usermod when reading the value from ArduinoJson.
   * - Pin values can be treated differently from an integer value by using the key name "pin"
   *   - "pin" can contain a single or array of integer values
   *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
   *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
   *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
   *
   * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
   *
   * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.
   * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
   * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
   *
   * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
   */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = enabled;
    // save these vars persistently whenever settings are saved
    top["autoStop"] = autoStopEnabled;
    top["milkerWashMode"] = milkerWashMode;
    top["parlorWashMode"] = parlorWashMode;
    top["waterChaserMode"] = waterChaserMode;
    top["hornMode"] = hornMode;
  }

  /*
   * readFromConfig() can be used to read back the custom settings you added with addToConfig().
   * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
   *
   * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
   * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
   * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
   *
   * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
   *
   * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
   * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
   *
   * This function is guaranteed to be called on boot, but could also be called every time settings are updated
   */
  bool readFromConfig(JsonObject &root)
  {
    // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
    // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

    JsonObject top = root[FPSTR(_name)];

    bool configComplete = !top.isNull();

    // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
    configComplete &= getJsonValue(top["autoStop"], autoStopEnabled, 1);
    configComplete &= getJsonValue(top["milkerWashMode"], milkerWashMode, MilkerWashMode(onFWD));
    configComplete &= getJsonValue(top["parlorWashMode"], parlorWashMode, ParlorWashMode(onFWDNoPrep));
    configComplete &= getJsonValue(top["waterChaserMode"], waterChaserMode, WaterChaserMode(onDetect));
    configComplete &= getJsonValue(top["hornMode"], hornMode, HornMode(stopFailure));

    return configComplete;
  }

  /*
   * appendConfigData() is called when user enters usermod settings page
   * it may add additional metadata for certain entry fields (adding drop down is possible)
   * be careful not to add too much as oappend() buffer is limited to 3k
   */
  void appendConfigData()
  {
    // oappend(SET_F("addInfo('"));
    // oappend(String(FPSTR(_name)).c_str());
    // oappend(SET_F(":great"));
    // oappend(SET_F("',1,'<i>(this is a great config value)</i>');"));
    // oappend(SET_F("addInfo('"));
    // oappend(String(FPSTR(_name)).c_str());
    // oappend(SET_F(":testString"));
    // oappend(SET_F("',1,'enter any string you want');"));
    // oappend(SET_F("dd=addDropdown('"));
    // oappend(String(FPSTR(_name)).c_str());
    // oappend(SET_F("','testInt');"));
    // oappend(SET_F("addOption(dd,'Nothing',0);"));
    // oappend(SET_F("addOption(dd,'Everything',42);"));
  }

#pragma endregion Config

  /**
   * handleButton() can be used to override default button behaviour. Returning true
   * will prevent button working in a default way.
   * Replicating button.cpp
   */

  bool handleButton(uint8_t b)
  {
    yield();
    // ignore certain button types as they may have other consequences
    if (!enabled || buttonType[b] == BTN_TYPE_NONE || buttonType[b] == BTN_TYPE_RESERVED || buttonType[b] == BTN_TYPE_PIR_SENSOR || buttonType[b] == BTN_TYPE_ANALOG || buttonType[b] == BTN_TYPE_ANALOG_INVERTED)
    {
      return false;
    }

    bool handled = false;
    // do your button handling here
    return handled;
  }
};

// add more strings here to reduce flash memory usage
const char ParlorControl::_name[] PROGMEM = "ParlorControl";
const char ParlorControl::_enabled[] PROGMEM = "enabled";

#pragma region Class Methods
#pragma region MQTT
#ifndef WLED_DISABLE_MQTT
/**
 * handling of MQTT message
 * topic only contains stripped topic (part after /wled/MAC)
 */
bool ParlorControl::onMqttMessage(char *topic, char *payload)
{
  // Handling specific topics
  if (strcmp(topic, PSTR("/cmnd/auto")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char autoStr[3];
      snprintf_P(autoStr, sizeof(autoStr), PSTR("%d"), autoStopEnabled);
      publishMqtt(autoStr, PSTR("/stat/auto"), true);
    }
    else
    {
      autoStopEnabled = atoi(payload);
      publishMqtt(payload, PSTR("/stat/auto"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/speed")) == 0)
  {
    char speedStr[10];
    snprintf_P(speedStr, sizeof(speedStr), PSTR("%.2f"), parlorSpeed);
    publishMqtt(speedStr, PSTR("/stat/speed"), true);
  }
  else if (strcmp(topic, PSTR("/cmnd/setspeed")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char setspeedStr[10];
      snprintf_P(setspeedStr, sizeof(setspeedStr), PSTR("%.2f"), speedSetpoint);
      publishMqtt(setspeedStr, PSTR("/stat/setspeed"), true);
    }
    else
    {
      speedSetpoint = atof(payload);
      publishMqtt(payload, PSTR("/stat/setspeed"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/stall")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char stallStr[3];
      snprintf_P(stallStr, sizeof(stallStr), PSTR("%d"), currentStall);
      publishMqtt(stallStr, PSTR("/stat/stall"), true);
    }
    else
    {
      currentStall = atoi(payload);
      publishMqtt(payload, PSTR("/stat/stall"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/water/exitwash")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char waterExitWashStr[3];
      snprintf_P(waterExitWashStr, sizeof(waterExitWashStr), PSTR("%d"), milkerWashMode);
      publishMqtt(waterExitWashStr, PSTR("/stat/water/exitwash"), true);
    }
    else
    {
      milkerWashMode = MilkerWashMode(atoi(payload));
      publishMqtt(payload, PSTR("/stat/water/exitwash"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/water/exitchase")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char waterExitChaseStr[3];
      snprintf_P(waterExitChaseStr, sizeof(waterExitChaseStr), PSTR("%d"), waterChaserMode);
      publishMqtt(waterExitChaseStr, PSTR("/stat/water/exitchase"), true);
    }
    else
    {
      waterChaserMode = WaterChaserMode(atoi(payload));
      publishMqtt(payload, PSTR("/stat/water/exitchase"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/water/parlorwash")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char parlorWashModeStr[3];
      snprintf_P(parlorWashModeStr, sizeof(parlorWashModeStr), PSTR("%d"), parlorWashMode);
      publishMqtt(parlorWashModeStr, PSTR("/stat/water/parlorwash"), true);
    }
    else
    {
      parlorWashMode = ParlorWashMode(atoi(payload));
      publishMqtt(payload, PSTR("/stat/water/parlorwash"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/cip")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char cipStr[3];
      snprintf_P(cipStr, sizeof(cipStr), PSTR("%d"), CIPStall);
      publishMqtt(cipStr, PSTR("/stat/cip"), true);
    }
    else
    {
      CIPStall = atoi(payload);
      publishMqtt(payload, PSTR("/stat/cip"), true); // Confirm success
      CIPUpdated = true;
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/group")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char groupStr[3];
      snprintf_P(groupStr, sizeof(groupStr), PSTR("%d"), groupsDone);
      publishMqtt(groupStr, PSTR("/stat/group"), true);
    }
    else
    {
      groupsDone = atoi(payload);
      publishMqtt(payload, PSTR("/stat/group"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/horn")) == 0)
  {
    if (strlen(payload) == 0)
    {
      char hornStr[3];
      snprintf_P(hornStr, sizeof(hornStr), PSTR("%d"), hornMode);
      publishMqtt(hornStr, PSTR("/stat/horn"), true);
    }
    else
    {
      hornMode = HornMode(atoi(payload));
      publishMqtt(payload, PSTR("/stat/horn"), true); // Confirm success
    }
  }
  else if (strcmp(topic, PSTR("/cmnd/mode")) == 0)
  {
    // This topic only publishes the current mode, no confirmation needed.
    char modeStr[3];
    snprintf_P(modeStr, sizeof(modeStr), PSTR("%d"), currentMode);
    publishMqtt(modeStr, PSTR("/stat/mode"), true);
  }
  else if (strcmp(topic, PSTR("/cmnd/ropeswitch")) == 0)
  {
    ropeSwitchCounter++;
    if (ropeSwitchCounter > 1000)
    {
      ropeSwitchCounter = 0;
    }
    char ropeStr[5];
    snprintf_P(ropeStr, sizeof(ropeStr), PSTR("%d"), ropeSwitchCounter);
    publishMqtt(ropeStr, PSTR("/stat/ropeswitch"), true);
    mb.Hreg(ropeReg, ropeSwitchCounter);
  }
  else if (strcmp(topic, PSTR("/cmnd/uptime")) == 0)
  {
    // This topic only publishes the current mode, no confirmation needed.
    char uptimeStr[12];
    snprintf_P(uptimeStr, sizeof(uptimeStr), PSTR("%lu"), millis());
    publishMqtt(uptimeStr, PSTR("/stat/uptime"), true);
  }

  // If none of the topics match, return false
  return false;
}

/**
 * onMqttConnect() is called when MQTT connection is established
 */
void ParlorControl::onMqttConnect(bool sessionPresent)
{
  // do any MQTT related initialisation here
  char subuf[38];
  if (mqttDeviceTopic[0] != 0)
  {
    strlcpy(subuf, mqttDeviceTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/cmnd/#"));
    mqtt->subscribe(subuf, 0);

    DEBUG_PRINTLN(PSTR("MQTT Connected"));
    publishMqtt(PSTR("Connected"), PSTR("/stat"), true);
  }
}
#endif

// implementation of non-inline member methods
void ParlorControl::publishMqtt(const char *state, const char *topic, bool retain)
{
#ifndef WLED_DISABLE_MQTT
  // Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED)
  {
    char subuf[38];
    strlcpy(subuf, mqttDeviceTopic, 33);
    // Use the topic argument to create a custom topic
    strcat(subuf, topic);
    DEBUG_PRINTLN(PSTR("MQTT Publishing: "));
    DEBUG_PRINTLN(topic);
    mqtt->publish(subuf, 0, retain, state);
    DEBUG_PRINTLN(PSTR("MQTT Published "));
    yield();
  }
#endif
}
#pragma endregion MQTT

#pragma region LED
void ParlorControl::setLEDRange(Light light, RgbColor color)
{
  byte start = -1;
  switch (light)
  {
  case autoStop:
    start = autoLedStart;
    break;
  case stall:
    start = stallLedStart;
    break;
  case cowFWD:
    start = cowFWDSafetyLedStart;
    break;
  case spare:
    start = spareLEDStart;
    break;

  default:
    break;
  }
  if (start > -1)
  {
    for (size_t i = start; i < start + 6; i++)
    {
      strip.SetPixelColor(i, color);
    }
    strip.Show();
  }
}

void ParlorControl::setLightColor(Light light, RgbColor color)
{
  if (!showRebootStatus)
  {

    if (light == spare)
    {
      if (spareLedState != color)
      {
        setLEDRange(light, color);
        spareLedState = color;
      }
    }

    else if (light == autoStop)
    {
      if (autoLedState != color)
      {
        setLEDRange(light, color);
        autoLedState = color;
      }
    }

    else if (light == stall)
    {
      if (stallLedState != color)
      {
        setLEDRange(light, color);
        stallLedState = color;
        lastStallLEDBlink = millis();
      }
    }
    else if (light == cowFWD)
    {
      if (cowFWDSafetyLedState != color)
      {
        setLEDRange(light, color);
        cowFWDSafetyLedState = color;
      }
    }
  }
}

void ParlorControl::hsvToRgb(byte h, byte s, byte v, byte &r, byte &g, byte &b)
{
  DEBUG_PRINTLN(PSTR("hsvToRgb"));
  DEBUG_PRINT(h);
  DEBUG_PRINT(PSTR(","));
  DEBUG_PRINT(s);
  DEBUG_PRINT(PSTR(","));
  DEBUG_PRINTLN(v);
  byte i = h / 43;                                    // 43 = 256 / 6 (number of color sextants)
  byte f = (h % 43) * 6;                              // 6 = 256 / 42 (width of each sextant)
  byte p = (v * (255 - s)) >> 8;                      // Equivalent to (v * (255 - s)) / 256
  byte q = (v * (255 - ((s * f) >> 8))) >> 8;         // Equivalent to (v * (255 - ((s * f) / 256))) / 256
  byte t = (v * (255 - ((s * (255 - f)) >> 8))) >> 8; // Equivalent to (v * (255 - ((s * (255 - f)) / 256))) / 256

  switch (i)
  {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  default:
    r = v;
    g = p;
    b = q;
    break;
  }
}

RgbColor ParlorControl::generateSpeedColor(float inputValue)
{
  DEBUG_PRINTLN(PSTR("Generating Speed Color"));
  int hueValue;
  if (inputValue < speedSetpoint - errorMargin)
  {
    hueValue = 0;
    DEBUG_PRINTLN(PSTR("<<"));
  }
  else if (inputValue > speedSetpoint + errorMargin)
  {
    hueValue = 170;
    DEBUG_PRINTLN(PSTR(">>"));
  }
  else
  {

    if (inputValue < speedSetpoint)
    {
      // Map the input value to a hue value between 0 (red) and 85 (green)
      hueValue = map(inputValue * 100, ((speedSetpoint - errorMargin) * 100), (speedSetpoint) * 100, 0, 85);
      DEBUG_PRINTLN(PSTR("<"));
    }
    else
    {
      // Map the input value to a hue value between 86 (green) and 170 (blue)
      hueValue = map(inputValue * 100, speedSetpoint * 100, (speedSetpoint + errorMargin) * 100, 86, 170);
      DEBUG_PRINTLN(PSTR(">"));
    }
  }

  byte red;
  byte green;
  byte blue;

  hsvToRgb(hueValue, 255, 255, red, green, blue);
  DEBUG_PRINTLN(PSTR("Done generating Speed Color"));

  return RgbColor(red, green, blue);
}
#pragma endregion LED

#pragma region Relay
void ParlorControl::setRelay(Relays relay, bool state)
{
  if (relay == parlor)
  {

    if (state == 1 && parlorStopRelayState == 0)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("parlorStop"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(parlorPauseRelayPin, HIGH);
      parlorStopRelayState = 1;
      lastParlorStopRelayTriggered = millis();
    }
    else if (state == 0 && parlorStopRelayState == 1)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("parlorStop"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(parlorPauseRelayPin, LOW);
      parlorStopRelayState = 0;
    }
  }
  else if (relay == horn)
  {

    if (state == 1 && hornRelayState == 0)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("horn"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(hornRelayPin, HIGH);
      hornRelayState = 1;
    }
    else if (state == 0 && hornRelayState == 1)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("horn"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(hornRelayPin, LOW);
      hornRelayState = 0;
    }
  }
  else if (relay == water)
  {

    if (state == 1 && waterRelayState == 0)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("chaser"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(waterRelayPin, HIGH);
      waterRelayState = 1;
    }
    else if (state == 0 && waterRelayState == 1)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("chaser"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(waterRelayPin, LOW);
      waterRelayState = 0;
    }
  }
  else if (relay == milkerWash)
  {

    if (state == 1 && milkerWashRelayState == 0)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("milkerWash"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(milkerWashRelayPin, HIGH);
      milkerWashRelayState = 1;
    }
    else if (state == 0 && milkerWashRelayState == 1)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("milkerWash"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(milkerWashRelayPin, LOW);
      milkerWashRelayState = 0;
    }
  }
  else if (relay == parlorWash)
  {

    if (state == 1 && parlorWashRelayState == 0)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("parlorWash"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(parlorWashRelayPin, HIGH);
      parlorWashRelayState = 1;
    }
    else if (state == 0 && parlorWashRelayState == 1)
    {
      DEBUG_PRINT(PSTR("Setting Relay:"));
      DEBUG_PRINTLN(PSTR("parlorWash"));
      DEBUG_PRINT(PSTR("State:"));
      DEBUG_PRINTLN(state);

      digitalWrite(parlorWashRelayPin, LOW);
      parlorWashRelayState = 0;
    }
  }
}
#pragma endregion Relay

#pragma region Speed Control
float ParlorControl::round_to_dp(float in_value, int decimal_place)
{
  DEBUG_PRINTLN(PSTR("Rounding to DP"));
  float multiplier = powf(10.0f, decimal_place);
  in_value = roundf(in_value * multiplier) / multiplier;
  return in_value;
}

void ParlorControl::getMedianSpeed()
{
  DEBUG_PRINTLN(PSTR("Getting Median Speed"));

  float y = (float(millis()) - float(lastStallChange)) / 1000.00;
  // float x = round_to_dp(y, 2);
  lastStallChange = millis();
  if (y > maxSecPerStall)
  {
    y = maxSecPerStall;
  }

  speedSamples.add(y);
  DEBUG_PRINT(PSTR("Current Speed="));
  DEBUG_PRINTLN(y);

  parlorSpeed = speedSamples.getMedian();
  timeToStallCenterPos = (parlorSpeed * 1000) * (1 - stallPosAtStallChange);

  // eliminate wrong time/speed calculations due to parlor stopping between stall and lengthening time between stalls
  if (timeToStallCenterPos < minOffsetMS)
  {
    timeToStallCenterPos = minOffsetMS;
  }

  else if (timeToStallCenterPos > maxOffsetMS)
  {
    timeToStallCenterPos = maxOffsetMS;
  }

  // DEBUG_PRINT("Parlor Speed=");
  // DEBUG_PRINTLN(parlorSpeed);
  // DEBUG_PRINT("Time To Stall Center Position="));
  // DEBUG_PRINTLN(timeToStallCenterPos);
  if (speedLEDEnabled)
  {
    setLightColor(spare, generateSpeedColor(parlorSpeed));
    lastSpeedLEDSet = millis();
    statusLedState = 1;
  }
  char speedStr[10];
  snprintf_P(speedStr, sizeof(speedStr), PSTR("%.2f"), parlorSpeed);
  publishMqtt(speedStr, PSTR("/stat/speed"), true);
  speedMeasured = 1;
  DEBUG_PRINTLN(PSTR("Done Getting Median Speed"));
}
#pragma endregion Speed Control

#pragma region Stall Functions
void ParlorControl::increment_stall()
{
  currentStall++;

  if (currentStall == 51)
  {
    currentStall = 1;
  }
}

void ParlorControl::stall_Change()
{
  stallChange = 1;
  increment_stall();
  digitalWrite(2, 1);
  if (stallChangeStatusLED)
  {
    setLightColor(spare, blue);
    lastStatusLEDBlink = millis();
    statusLedState = 1;
  }
  if (millis() - lastStallChange > checkCIPInterval)
  {
    CIPUpdated = false;
  }
  if (!CIPUpdated)
  {
    publishMqtt("", PSTR("/get/cip"), true);
  }

  speedMeasured = 0;
  DEBUG_PRINTLN(PSTR("Stall Change"));
}
#pragma endregion Stall Functions

void ParlorControl::resetStats()
{
  if (!statsReset)
  {
    totalFullStalls = 0;
    totalEmptyStallStops = 0;
    totalFwdSftStops = 0;
    totalStopFailures = 0;
    totalSafetySwitchTriggers = 0;
    for (size_t i = 0; i < 6; i++)
    {
      totalInGroup[i] = 0;
    }

    avgTimePerStall = 0;
    startTime = time_t(0);
    endTime = time_t(0);
    setGroup(0);
    completed = 1;
    statsReset = 1;
  }
}

void ParlorControl::setMode(AutoMode mode)
{

  AutoMode oldMode = currentMode;
  currentMode = mode;

  if (oldMode != currentMode)
  {
    DEBUG_PRINTLN(PSTR("Setting Mode"));
    char modeStr[3];
    snprintf_P(modeStr, sizeof(modeStr), PSTR("%d"), currentMode);
    publishMqtt(modeStr, PSTR("/stat/mode"), true);
  }
}

void ParlorControl::setGroup(byte group)
{
  byte oldGroup = groupsDone;
  groupsDone = group;

  if (oldGroup != groupsDone)
  {
    char groupStr[3];
    snprintf_P(groupStr, sizeof(groupStr), PSTR("%d"), groupsDone);
    publishMqtt(groupStr, PSTR("/stat/group"), true);
  }
}

#pragma endregion Class Methods
