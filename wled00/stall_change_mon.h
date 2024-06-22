#pragma once

 #define SIMULATE

#include "wled.h"
#include <ModbusIP_ESP8266.h>

bool ropeSwitch = 0;
bool initRope = 0;
uint16_t cbRope(TRegister *reg, uint16_t val)
{
  // Check if Coil state is going to be changed
  if (reg->value != val && initRope != 0)
  {
    Serial.print("Set reg: ");
    Serial.println(val);
    ropeSwitch = 1;
  }
  initRope = 1;
  return val;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress ip)
{
  Serial.println(ip);
  return true;
}

class StallChangeMon : public Usermod
{
private:
#pragma region variables
#define LED_BUILTIN 2

  // config
  bool enableStatusLED = 1;

  // Modbus register addresses
  const uint8_t stallReg = 100;
  const uint8_t toggleReg = 101;
  const uint8_t idReg = 102;
  const uint8_t ropeReg = 103;

  ModbusIP mb; // ModbusTCP object

  ulong stallChangeCounter = 0;
  ulong buttonPressCounter = 0;
  ulong ropeSwitchCounter = 0;

  // default timing settings
  const int connectedStatusLedRate = 1000;   // Blink rate when connected (in milliseconds)
  const int disconnectedStatusLedRate = 100; // Blink rate when disconnected (in milliseconds)
  const int ropeSwitchStatusLedRate = 25;    // Blink rate when disconnected (in milliseconds)
  int statusLedRate = disconnectedStatusLedRate;
  const int statusLedOnTime = 1000;
  const int stallLedOnTime = 1000;
  const int IDLedOnTime = 1000;
  const int ropeSwitchOnTime = 1000;
  const int minDelayBetweenStalls = 750;
  const int maxDelayBetweenID_StallChange = 3500;
  const int IDBeforeStallWaitTime = 4000;
  const byte modbusReadInterval = 20;
  const byte modbusPullInterval = 20;
  const byte checkInterval = 5;
  const int timeBetweenButtonPress = 1000;
  const int LEDAltBlink = 500;

  // input pin assignments
  const byte stallChangePin = 23;
  const byte IDChangePin = 22;
  const byte buttonPin = 21;

  // output pin assignments
  const byte stallLedPin = 16;
  const byte IDLedPin = 14;
  const byte noIDLedPin = 12;
  const byte ropeTrigLedPin = 13;
  const byte relayPin = 15;

  // LED Status
  bool StallLedOn = 0;
  bool IDLedOn = 0;
  bool NoIDLedOn = 0;
  bool autoLedOn = 0;

  // states
  bool ropeTriggered = 0;
  bool parlorConnected = 0;
  bool stallChange = 0;
  bool IDChange = 0;
  bool buttonPress = 0;
  bool statusLEDState = 0;
  bool NoID_Mode = 1;

  // time of last...
  ulong lastStallChange = 0;
  ulong lastInputRead = 0;
  ulong lastNoID = 0;
  ulong RSTriggerTime = 0;
  ulong lastButtonPress = 0;
  ulong lastLEDBlink = 0;
  ulong lastStatusLEDBlink = 0;
  ulong lastIDChange = 0;
  ulong lastSimStallChange = 0;
  ulong lastModbusTask = 0;
  ulong lastModbusRead = 0;
  ulong lastModbusPull = 0;
#pragma endregion variables

#pragma region class methods
  // uint16_t cbRope(TRegister *reg, uint16_t val);
  // bool cbConn(IPAddress ip);
  void activateRopeSwitch();
  bool simulateStallChange(int sec);
  void writeRegister(uint16_t reg, uint16_t value);
#pragma endregion class methods

public:
  void setup()
  {
    Serial.begin(115200);

    // init inputs
    pinMode(stallChangePin, INPUT);
    pinMode(IDChangePin, INPUT);
    pinMode(buttonPin, INPUT);

    // init outputs
    pinMode(IDLedPin, OUTPUT);
    pinMode(stallLedPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(noIDLedPin, OUTPUT);
    pinMode(ropeTrigLedPin, OUTPUT);
    pinMode(relayPin, OUTPUT);

    // init setting
    digitalWrite(relayPin, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  }

  void connected()
  {
    parlorConnected = 1;
    mb.onConnect(cbConn); // Add callback on connection event
    mb.connect(remote);   // Try to connect if no connection
    mb.client();
    mb.addHreg(ropeReg, 0, 1);
    mb.onSetHreg(ropeReg, cbRope); // Add callback on Coil LED_COIL value set
  }

  void loop()
  {

    if (millis() - lastModbusTask > modbusReadInterval)
    {
      mb.task(); // Common local Modbus task
      lastModbusTask = millis();
    }

    if (millis() - lastModbusPull > modbusPullInterval)
    {
      if (mb.isConnected(remote))
      {                                        // Check if connection to Modbus Slave is established
        mb.pullHreg(remote, ropeReg, ropeReg); // Initiate Read Coil from Modbus Slave
      }
      else
      {
        mb.connect(remote); // Try to connect if no connection
      }
      lastModbusPull = millis();
    }

    // // Modbus read registers for changes
    // if (millis() - lastModbusRead > modbusReadInterval)
    // {
    //   uint16_t lastRopeSwitchCount = mb.Hreg(ropeReg);
    //   if (lastRopeSwitchCount != ropeSwitchCounter)
    //   {
    //     digitalWrite(ropeTrigLedPin, HIGH);
    //     digitalWrite(relayPin, HIGH);

    //     ropeTriggered = 1;
    //     RSTriggerTime = millis();

    //     Serial.println("Rope Switch On");
    //   }

    //   ropeSwitchCounter = lastRopeSwitchCount;
    //   lastModbusRead = millis();
    // }

    if (ropeSwitch)
    {
      activateRopeSwitch();
    }

    // run every x millis (default: 5)
    if (millis() - lastInputRead > checkInterval)
    {
#ifdef SIMULATE
      // simulate stall change with speed of +-1 of supplied value
      stallChange = simulateStallChange(10);
#else
      // check for stall change
      stallChange = digitalRead(stallChangePin);
#endif
      if (stallChange == HIGH && millis() - lastStallChange > minDelayBetweenStalls)
      {
        Serial.println("Stall Change");
        StallLedOn = 1;
        digitalWrite(stallLedPin, HIGH); // turn the LED on (HIGH is the voltage level)
        if (parlorConnected)
        {
          stallChangeCounter++;
          if (stallChangeCounter > 1000)
          {
            stallChangeCounter = 0;
          }

          writeRegister(stallReg, stallChangeCounter); // set stall led register to counter value
        }

        lastStallChange = millis();
      }
      else if (StallLedOn == 1)
      {
        if (millis() - lastStallChange > stallLedOnTime)
        {
          StallLedOn = 0;
          digitalWrite(stallLedPin, LOW); // turn the LED off by making the voltage LOW
        }
      }

      // check for ID change
      IDChange = digitalRead(IDChangePin);
      // IDChange = false;
      if (IDChange == HIGH && millis() - lastIDChange > minDelayBetweenStalls)
      {
        if (parlorConnected)
        {
          // parlorRequest.sendHttpRequest(IDChangeUrl,"POST");
        }
        IDLedOn = 1;
        digitalWrite(IDLedPin, HIGH); // turn the LED on (HIGH is the voltage level)
        lastIDChange = millis();
        NoID_Mode = 0;
      }
      else if (IDLedOn == 1)
      {
        if (millis() - lastIDChange > IDLedOnTime)
        {
          IDLedOn = 0;
          digitalWrite(IDLedPin, LOW); // turn the LED off by making the voltage LOW
        }
      }

      buttonPress = digitalRead(buttonPin);
      // IDChange = false;
      if (buttonPress == HIGH && millis() - lastButtonPress > timeBetweenButtonPress)
      {
        if (parlorConnected)
        {
          buttonPressCounter++;
          if (buttonPressCounter > 1000)
            buttonPressCounter = 0;
          writeRegister(toggleReg, buttonPressCounter); // set toggle register to counter value
        }

        Serial.println("Button Press");
        lastButtonPress = millis();

        digitalWrite(ropeTrigLedPin, HIGH);
        digitalWrite(relayPin, HIGH);

        NoID_Mode = 1;
        ropeTriggered = 1;
        lastNoID = millis();
        RSTriggerTime = millis();

        Serial.println("Rope Switch On");
        Serial.println("No ID LED On");
      }

      lastInputRead = millis();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      parlorConnected = 1;
    }
    else
    {
      parlorConnected = 0;
    }
    // if (enableStatusLED == 1 && millis() - lastStatusLEDBlink > statusLedOnTime)
    // {
    //   if (statusLEDState == 0)
    //   {
    //     digitalWrite(LED_BUILTIN, HIGH);
    //     statusLEDState = 1;
    //   }
    //   else
    //   {
    //     digitalWrite(LED_BUILTIN, LOW);
    //     statusLEDState = 0;
    //   }
    //   lastStatusLEDBlink = millis();
    // }

    // turn off rope switch after set delay
    if (ropeTriggered == 1 && millis() - RSTriggerTime > ropeSwitchOnTime)
    {
      ropeTriggered = 0;
      digitalWrite(ropeTrigLedPin, LOW);
      digitalWrite(relayPin, LOW);
      Serial.println("Rope Switch Off");
    }

    if (ropeTriggered)
    {
      statusLedRate = ropeSwitchStatusLedRate;
    }
    else if (parlorConnected)
    {
      statusLedRate = connectedStatusLedRate;
    }
    else
    {
      statusLedRate = disconnectedStatusLedRate;
    }

    // blink status LED
    if (enableStatusLED == 1 && millis() - lastStatusLEDBlink > statusLedRate)
    {
      digitalWrite(LED_BUILTIN, statusLEDState == 0 ? HIGH : LOW);
      statusLEDState = !statusLEDState; // Toggle the LED state
      lastStatusLEDBlink = millis();    // Update the last blink time
    }
  }
};

// // Callback function for write (set) Coil. Returns value to store.
// uint16_t StallChangeMon::cbRope(TRegister *reg, uint16_t val)
// {
//   digitalWrite(ropeTrigLedPin, HIGH);
//   digitalWrite(relayPin, HIGH);

//   ropeTriggered = 1;
//   RSTriggerTime = millis();

//   Serial.println("Rope Switch On");
//   return val;
// }

// // Callback function for client connect. Returns true to allow connection.
// bool StallChangeMon::cbConn(IPAddress ip)
// {
//   Serial.println(ip);
//   return true;
// }
void StallChangeMon::activateRopeSwitch()
{
  digitalWrite(ropeTrigLedPin, HIGH);
  digitalWrite(relayPin, HIGH);

  ropeTriggered = 1;
  ropeSwitch = 0;
  RSTriggerTime = millis();

  Serial.println("Rope Switch On");
}
void StallChangeMon::writeRegister(uint16_t reg, uint16_t value)
{
  if (mb.isConnected(remote))
  { // Check if connection to Modbus Slave is established
    mb.writeHreg(remote, reg, value);
    Serial.println("Completed Write");
  }
  else
  {
    Serial.println("No Connection");
    mb.connect(remote); // Try to connect if no connection
  }
}

// simulate stall change with speed of +-1 of supplied value
bool StallChangeMon::simulateStallChange(int sec)
{
  float variation = .1 * random(-10, 10);
  int speed = (sec + variation) * 1000;

  // DEBUG_PRINT("Var:");
  // DEBUG_PRINTLN(variation);
  // DEBUG_PRINT("Speed:");
  // DEBUG_PRINTLN(speed);
  if (millis() - lastSimStallChange > speed)
  {
    lastSimStallChange = millis();
    return true;
  }
  else
  {
    return false;
  }
}
