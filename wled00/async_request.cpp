#include "wled.h"
#if !(defined(ESP8266) || defined(ESP32))
#error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#define ASYNC_HTTPS_REQUEST_GENERIC_VERSION_MIN_TARGET "AsyncHTTPSRequest_Generic v2.2.1"
#define ASYNC_HTTPS_REQUEST_GENERIC_VERSION_MIN 2002001

/////////////////////////////////////////////////////////

// Seconds for timeout, default is 3s
#define DEFAULT_RX_TIMEOUT 10

// Uncomment for certain HTTP site to optimize
// #define NOT_SEND_HEADER_AFTER_CONNECTED        true

// Level from 0-4
#define ASYNC_HTTPS_DEBUG_PORT Serial

#define _ASYNC_TCP_SSL_LOGLEVEL_ 1
#define _ASYNC_HTTPS_LOGLEVEL_ 1

// Use larger queue size if necessary for large data transfer. Default is 512 bytes if not defined here
// #define ASYNC_QUEUE_LENGTH     512

// Use larger priority if necessary. Default is 10 if not defined here. Must be > 4 or adjusted to 4
// #define CONFIG_ASYNC_TCP_PRIORITY   (12)

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#ifdef HTTPSOnly
#include <AsyncHTTPSRequest_Generic.h> // https://github.com/khoih-prog/AsyncHTTPSRequest_Generic
AsyncHTTPSRequest httpsRequest;
void sendHttpsRequest(const char *method, const char *URL)
{
  static bool requestOpenResult;

  if (httpsRequest.readyState() == readyStateUnsent || httpsRequest.readyState() == readyStateDone)
  {
    // requestOpenResult = httpsRequest.open("GET", "https://worldtimeapi.org/api/timezone/Europe/London.txt");
    // requestOpenResult = httpsRequest.open("GET", "https://worldtimeapi.org/api/timezone/America/Toronto.txt");
    requestOpenResult = httpsRequest.open("GET", "https://worldtimeapi.org/api/timezone/America/Chicago.txt");

    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      httpsRequest.send();
    }
    else
    {
      Serial.println(F("Can't send bad request"));
    }
  }
  else
  {
    Serial.println(F("Can't send request"));
  }
}

void httpsRequestCB(void *optParm, AsyncHTTPSRequest *request, int readyState)
{
  (void)optParm;

  if (readyState == readyStateDone)
  {
    AHTTPS_LOGDEBUG0(F("\n**************************************\n"));
    AHTTPS_LOGDEBUG1(F("Response Code = "), request->responseHTTPString());

    if (request->responseHTTPcode() == 200)
    {
      Serial.println(F("\n**************************************"));
      Serial.println(request->responseText());
      Serial.println(F("**************************************"));
    }

    request->setDebug(false);
  }
}
#else

#include <AsyncHTTPRequest_Generic.h> // https://github.com/khoih-prog/AsyncHTTPRequest_Generic
AsyncHTTPRequest httpRequest;

void sendHttpRequest(const char* method, const char* URL)
{
  static bool requestOpenResult;

  if (httpRequest.readyState() == readyStateUnsent || httpRequest.readyState() == readyStateDone)
  {
    // requestOpenResult = request.open("GET", "http://worldtimeapi.org/api/timezone/Europe/London.txt");
    requestOpenResult = httpRequest.open("GET", "http://worldtimeapi.org/api/timezone/America/Toronto.txt");

    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      httpRequest.send();
    }
    else
    {
      Serial.println(F("Can't send bad request"));
    }
  }
  else
  {
    Serial.println(F("Can't send request"));
  }
}

void httpRequestCB(void *optParm, AsyncHTTPRequest *httpRequest, int readyState)
{
  (void)optParm;

  if (readyState == readyStateDone)
  {
    AHTTP_LOGDEBUG(F("\n**************************************"));
    AHTTP_LOGDEBUG1(F("Response Code = "), httpRequest->responseHTTPString());

    if (httpRequest->responseHTTPcode() == 200)
    {
      Serial.println(F("\n**************************************"));
      Serial.println(httpRequest->responseText());
      Serial.println(F("**************************************"));
    }
  }
}
#endif

// gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void asyncRequestSetup()
{
#ifdef HTTPSOnly
  httpsRequest.setDebug(false);
  httpsRequest.onReadyStateChange(httpsRequestCB);
#else
  httpRequest.setDebug(false);
  httpRequest.onReadyStateChange(httpRequestCB);
#endif
}
