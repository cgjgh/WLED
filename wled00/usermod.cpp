#include "wled.h"
#include <NeoPixelBus.h>
#include <RunningMedian.h>

#pragma region variables
bool autoStopEnabled = 1;
bool enableStatusLED = 1;

const int statusLedOnTime = 1000;
const int stallLedOnTime = 1000;
const int minDelayBetweenStalls = 750;
const int inputReadInterval = 5;

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
const byte relay3 = 27;
const byte relay4 = 26;

// input pin assignments
const byte stallSensorPin = input1;
const byte bypassSwitchPin = input2;
const byte cowFWDsafetySensorPin = input3;

// relay pin assignments
const byte parlorPauseRelayPin = relay1;
const byte waterRelayPin = relay2;

// relay states
bool parlorStopRelayState = 0;
bool waterRelayState = 0;

// millis() time of last...
long lastStallChange = 0;
long lastLEDBlink = 0;
long lastStatusLEDBlink = 0;
long lastStallLEDBlink = 0;
long lastInputRead = 0;

// parlor vars
bool stallChange = 0;
bool IDChange = 0;
float parlorSpeed = 0;
bool stallEmpty = 0;
bool cowFWDSaftey = 0;

// misc
bool speedMeasured = 0;
long timeToStallCenterPos = 0;

#pragma endregion

// leds
#pragma region leds

const uint16_t PixelCount = 24;
const uint8_t PixelPin = 0;

#define colorSaturation 255

// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor orange(colorSaturation, 80, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

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

void setLEDRange(Light light, RgbColor color)
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

void setLightColor(Light light, RgbColor color)
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
#pragma endregion

// relays
#pragma region relay
enum Relays
{
  parlor = 1,
  water = 2,
};

void setRelay(Relays relay, bool state)
{
  if (relay == parlor)
  {
    if (state == 1 && parlorStopRelayState == 0)
    {
      digitalWrite(parlorPauseRelayPin, HIGH);
      parlorStopRelayState = 1;
    }
    else if (state == 0 && parlorStopRelayState == 1)
    {
      digitalWrite(parlorPauseRelayPin, LOW);
      parlorStopRelayState = 0;
    }
  }
  else if (relay == water)
  {
    if (state == 1 && waterRelayState == 0)
    {
      digitalWrite(waterRelayPin, HIGH);
      waterRelayState = 1;
    }
    else if (state == 0 && waterRelayState == 1)
    {
      digitalWrite(waterRelayPin, LOW);
      waterRelayState = 0;
    }
  }
}
#pragma endregion

#pragma region Async Web Request

//AsyncHTTPSWebRequest sheetsRequest(sheetsRequestCB, "", "GET");
AsyncHTTPWebRequest controlRequest(controlRequestCB, "", "GET");

#pragma endregion

#pragma region Speed -Timing

// array to calculate running median
RunningMedian speedSamples = RunningMedian(5);

// round float to x decimal places
float round_to_dp(float in_value, int decimal_place)
{
  float multiplier = powf(10.0f, decimal_place);
  in_value = roundf(in_value * multiplier) / multiplier;
  return in_value;
}

// calculate the median speed from millis()
void getMedianSpeed()
{
  float y = (millis() - lastStallChange) / 1000;
  float x = round_to_dp(y, 2);
  lastStallChange = millis();
  speedSamples.add(y);
  DEBUG_PRINT("Current Speed=");
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

  DEBUG_PRINT("Parlor Speed=");
  DEBUG_PRINTLN(parlorSpeed);
  DEBUG_PRINT("Time To Stall Center Position=");
  DEBUG_PRINTLN(timeToStallCenterPos);
  speedMeasured = 1;
}
#pragma endregion

#pragma region web page

const char *PARAM_VIRTUAL = "virtualState";
const char *PARAM_AUTO = "autoState";
const char *PARAM_AUTOID = "autoIDState";
const char *PARAM_STALL = "stallState";
const char *PARAM_STALLPOS = "stallPosition";

bool useVirtualAutoSwitch = 0;
bool virtualAutoSwitchState = 0;
bool virtualAutoIDSwitchState = 0;
bool stallAlwaysOn = 0;

const char auto_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML>
<html>

<head>
  <title>Parlor Auto Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }

    h2 {
      font-size: 3.0rem;
    }

    p {
      font-size: 3.0rem;
    }

    body {
      font-family: Verdana, Helvetica, sans-serif;
      text-align: center;
      background-color: #222;
      margin: 0;
      color: #fff;
    }

    .switch {
      position: relative;
      display: inline-block;
      width: 120px;
      height: 68px
    }

    .switch input {
      display: none
    }

    .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      border-radius: 34px
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 52px;
      width: 52px;
      left: 8px;
      bottom: 8px;
      background-color: #fff;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 68px
    }

    input:checked+.slider {
      background-color: #2196F3
    }

    input:checked+.slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px)
    }
     @import url("style.css");
  </style>
</head>

<body>
  <h2>Parlor Auto Mode</h2>
  %BUTTONPLACEHOLDER%
  %NUMBERPLACEHOLDER%
<button type="submit" onclick="rope()">Rope Switch</button>

  <script>
  
  function rope() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/ropeswitch", true);
      xhr.send();
    }
  
  function toggleCheckbox(element) {
      var xhr = new XMLHttpRequest();
      if (element.id == "virtual") {

        if (element.checked) { xhr.open("GET", "/autoset?virtualState=1", true); }
        else { xhr.open("GET", "/autoset?virtualState=0", true); }
        xhr.send();
      }

      else if (element.id == "auto") {

        if (element.checked) { xhr.open("GET", "/autoset?autoState=1", true); }
        else { xhr.open("GET", "/autoset?autoState=0", true); }
        xhr.send();
      }

      else if (element.id == "autoID") {

        if (element.checked) { xhr.open("GET", "/autoset?autoIDState=1", true); }
        else { xhr.open("GET", "/autoset?autoIDState=0", true); }
        xhr.send();
      }
      else if (element.id == "stall") {

        if (element.checked) { xhr.open("GET", "/autoset?stallState=1", true); }
        else { xhr.open("GET", "/autoset?stallState=0", true); }
        xhr.send();
      }
    }

    function numberChanged(element) {
      var xhr = new XMLHttpRequest();
      if (element.id == "stallPosition") {

         xhr.open("GET", "/autoset?stallPosition=" + element.value, true); 
        xhr.send();
      }
    }


    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          var inputChecked;
          var outputStateM;
          if (this.responseText == 1) {
            inputChecked = true;
            outputStateM = "On";
          }
          else {
            inputChecked = false;
            outputStateM = "Off";
          }
          document.getElementById("virtual").checked = inputChecked;
          document.getElementById("useVirtualState").innerHTML = outputStateM;
        }
      };
      xhttp.open("GET", "/autostate?virtualState=1", true);
      xhttp.send();
    }, 1000);

    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          var inputChecked;
          var outputStateM;
          if (this.responseText == 1) {
            inputChecked = true;
            outputStateM = "On";
          }
          else {
            inputChecked = false;
            outputStateM = "Off";
          }
          document.getElementById("auto").checked = inputChecked;
          document.getElementById("autoState").innerHTML = outputStateM;
        }
      };
      xhttp.open("GET", "/autostate?autoState=1", true);
      xhttp.send();
    }, 1000);

    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          var inputChecked;
          var outputStateM;
          if (this.responseText == 1) {
            inputChecked = true;
            outputStateM = "On";
          }
          else {
            inputChecked = false;
            outputStateM = "Off";
          }
          document.getElementById("autoID").checked = inputChecked;
          document.getElementById("autoIDState").innerHTML = outputStateM;
        }
      };
      xhttp.open("GET", "/autostate?autoIDState=1", true);
      xhttp.send();
    }, 1000);
      
      setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          var inputChecked;
          var outputStateM;
          if (this.responseText == 1) {
            inputChecked = true;
            outputStateM = "On";
          }
          else {
            inputChecked = false;
            outputStateM = "Off";
          }
          document.getElementById("stall").checked = inputChecked;
          document.getElementById("stallState").innerHTML = outputStateM;
        }
      };
      xhttp.open("GET", "/autostate?stallState=1", true);
      xhttp.send();
    }, 1000);
  </script>
</body>

</html>
)rawliteral";

String autoState()
{
  if (virtualAutoSwitchState)
  {
    return "checked";
  }
  else
  {
    return "";
  }
  return "";
}

String autoIDState()
{
  if (virtualAutoIDSwitchState)
  {
    return "checked";
  }
  else
  {
    return "";
  }
  return "";
}

String virtualState()
{
  if (useVirtualAutoSwitch)
  {
    return "checked";
  }
  else
  {
    return "";
  }
  return "";
}

String stallState()
{
  if (stallAlwaysOn)
  {
    return "checked";
  }
  else
  {
    return "";
  }
  return "";
}

// Replaces placeholder with button section in your web page
String processor(const String &var)
{
  // Serial.println(var);
  if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";

    String useVirtualValue = virtualState();
    buttons += "<h4>Use Virtual Auto Switch - <span id=\"useVirtualState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"virtual\" " + useVirtualValue + "><span class=\"slider\"></span></label>";

    String autoStateValue = autoState();
    buttons += "<h4>Auto Mode - <span id=\"autoState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"auto\" " + autoStateValue + "><span class=\"slider\"></span></label>";

    String autoIDStateValue = autoIDState();
    buttons += "<h4>Auto With ID Mode - <span id=\"autoIDState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"autoID\" " + autoIDStateValue + "><span class=\"slider\"></span></label>";
    String stallStateValue = stallState();
    buttons += "<h4>Stall Always On - <span id=\"stallState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"stall\" " + stallStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  if (var == "NUMBERPLACEHOLDER")
  {
    String number = "";

    String stallPosition = String((int8_t)(stallPosAtStallChange * 100)).c_str();
    number += "<h4>Stall Position at Stall Change(%)<span id=\"stallPosition1\"></span></h4><label class=\"number\"><input type=\"number\" onchange=\"numberChanged(this)\" id=\"stallPosition\" value=\"" + stallPosition + "\"><span class=\"numInput\"></span></label>";

    return number;
  }
  return String();
}
#pragma endregion

void stall_Change()
{
  stallChange = 1;
  digitalWrite(2, 1);
  setLightColor(spare, blue);

  lastStatusLEDBlink = millis();
  statusLedState = 1;
  speedMeasured = 0;
  DEBUG_PRINTLN("Stall Change");
}

// gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  // initialize inputs
  pinMode(stallSensorPin, INPUT);
  pinMode(bypassSwitchPin, INPUT);
  pinMode(cowFWDsafetySensorPin, INPUT);

  // initialize Light

  strip.Begin();
  strip.Show();

  // initialize relay pin outputs
  pinMode(parlorPauseRelayPin, OUTPUT);
  pinMode(waterRelayPin, OUTPUT);

  // initial setting
  digitalWrite(parlorPauseRelayPin, LOW);
  digitalWrite(waterRelayPin, LOW);
  digitalWrite(2, LOW);
}

char autoStop1Url[40] = "http://4.3.2.2/autostopon";
char autoStop0Url[40] = "http://4.3.2.2/autostopoff";
char ropeSwitchUrl[40] = "http://4.3.2.2/ropeswitch";

// char autoStop1Url[40] = "http://4.3.2.4/autostopon";
// char autoStop0Url[40] = "http://4.3.2.4/autostopoff";
// char ropeSwitchUrl[40] = "http://4.3.2.4/ropeswitch";

// custom webpages and commands
void userWebServerSetup()
{
  server.on("/stallchange", HTTP_POST, [](AsyncWebServerRequest *request)
            { stall_Change(); });

  server.on("/IDchange", HTTP_POST, [](AsyncWebServerRequest *request) {});

  server.on("/ropeswitch", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
    Serial.println("ropeswitch");

controlRequest.sendHttpRequest(ropeSwitchUrl,"POST"); });

  // Route for root / web page
  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", auto_html, processor); });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/autoset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
   Serial.print("autoset ");
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_VIRTUAL)) {
   Serial.print("virtual=");

      inputMessage = request->getParam(PARAM_VIRTUAL)->value();
      inputParam = PARAM_VIRTUAL;
      useVirtualAutoSwitch = inputMessage.toInt();
    }
    else if (request->hasParam(PARAM_AUTO)) {
   Serial.print("auto=");
      inputMessage = request->getParam(PARAM_AUTO)->value();
      inputParam = PARAM_AUTO;
      virtualAutoSwitchState = inputMessage.toInt();
      request->send(200, "text/plain", String(virtualAutoSwitchState).c_str());
    }
    else if (request->hasParam(PARAM_AUTOID)) {
   Serial.print("autoID=");
      inputMessage = request->getParam(PARAM_AUTOID)->value();
      inputParam = PARAM_AUTOID;
      virtualAutoIDSwitchState = inputMessage.toInt();
      if (useVirtualAutoSwitch)
      {
       if (virtualAutoIDSwitchState)
       {
          controlRequest.sendHttpRequest(autoStop1Url,"POST");
          Serial.println("autostop 1");
       }

       else {
          controlRequest.sendHttpRequest(autoStop0Url,"POST");
          Serial.println("autostop 0");

       }

      }
      
    }
    
    else if (request->hasParam(PARAM_STALL)) {
   Serial.print("stall=");
      inputMessage = request->getParam(PARAM_STALL)->value();
      inputParam = PARAM_STALL;
      stallAlwaysOn = inputMessage.toInt();
    }

    else if (request->hasParam(PARAM_STALLPOS)) {
   Serial.print("stallPosition=");
      inputMessage = request->getParam(PARAM_STALLPOS)->value();
      inputParam = PARAM_STALLPOS;
      stallPosAtStallChange = (float)inputMessage.toInt() / (float)100;
    }

    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK"); });

  // Send a GET request to <ESP_IP>/state
  server.on("/autostate", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_VIRTUAL)) {
                    request->send(200, "text/plain", String(useVirtualAutoSwitch).c_str());

    }
    else if (request->hasParam(PARAM_AUTO)) {
                    request->send(200, "text/plain", String(virtualAutoSwitchState).c_str());

    }
    else if (request->hasParam(PARAM_AUTOID))
    {
      controlRequest.sendHttpRequest("http://4.3.2.2/autostopstate", "GET");
      request->send(200, "text/plain", String(virtualAutoIDSwitchState).c_str());
    }
     else if (request->hasParam(PARAM_STALL)) {
               request->send(200, "text/plain", String(stallAlwaysOn).c_str());
    }
    else {
          request->send(200, "text/plain", "");
    } });
}

// gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
}

long lastTime = 0;

void controlRequestCB(void *optParm, AsyncHTTPRequest *request, int readyState)
{
  controlRequest.httpRequestCB(optParm, request, readyState);

  if (controlRequest.newResponse)
  {
    virtualAutoIDSwitchState = controlRequest.response().toInt();
  };
}

void sheetsRequestCB(void *optParm, AsyncHTTPRequest *request, int readyState)
{
 // controlRequest.httpRequestCB(optParm, request, readyState);
}

// loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  if (millis() - lastInputRead > inputReadInterval)
  {
    // check if bypass switch is on
    //autoStopEnabled = digitalRead(bypassSwitchPin);

    // if (useVirtualAutoSwitch)
    // {
      autoStopEnabled = virtualAutoSwitchState;
    //}

    // check if stall status has changed
    stallEmpty = digitalRead(stallSensorPin);

    if (autoStopEnabled)
    {
      setLightColor(autoStop, green);

      if (!stallEmpty)
      {
        if (!(stallLedState == black))
        {
          setLightColor(stall, green);
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
    cowFWDSaftey = digitalRead(cowFWDsafetySensorPin);
    if (!cowFWDSaftey)
    {
      setLightColor(cowFWD, red);
      setRelay(water, (1));
    }
    else
    {
      setLightColor(cowFWD, black);
      setRelay(water, (0));
    }
  }

  if (stallChange)
  {
    if (!speedMeasured)
    {
      getMedianSpeed();
    }

    if (millis() > (lastStallChange + timeToStallCenterPos))
    {
      stallEmpty = digitalRead(stallSensorPin);
      if (!stallEmpty)
      {
        setLightColor(stall, green);

        setRelay(parlor, 0);
      }
      else
      {

        if (autoStopEnabled)
        {
          setLightColor(stall, red);
          setRelay(parlor, 1);
        }
        else
        {
          setLightColor(stall, orange);
          setRelay(parlor, 0);
        }
      }
      stallChange = 0;
    }
  }

  if (parlorStopRelayState)
  {
    stallEmpty = digitalRead(stallSensorPin);
    if (!stallEmpty)
    {
      setLightColor(stall, green);

      setRelay(parlor, 0);
    }
  }

  if (millis() - lastStatusLEDBlink > 1000 && spareLedState == blue)
  {
    digitalWrite(2, 0);
    setLightColor(spare, black);
  }

  if (!stallAlwaysOn)
  {

    // if (stallLedState == red)
    // {
    //     if (millis() - lastStallLEDBlink > 8000)
    //     {
    //         setLightColor(stall, black);
    //     }
    // }

    if (stallLedState == green || stallLedState == orange)
    {
      if (millis() - lastStallLEDBlink > 4000)
      {
        setLightColor(stall, black);
      }
    }
  }

  if (millis() - lastTime > 5000)
  {
    lastTime = millis();
  }
}
