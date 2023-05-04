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

void AsyncHTTPSWebRequest::httpsRequestCB(void *optParm, AsyncHTTPSRequest *request, int readyState)
{
  (void)optParm;

  if (readyState == readyStateDone)
  {
    if (request->responseHTTPcode() == 200)
    {
      responseStr = request->responseText();
      newResponse = 1;
    }

    request->setDebug(false);
  }
}

void AsyncHTTPSWebRequest::sendHttpsRequest(const char *URL, const char *method)
{
  static bool requestOpenResult;

  if (httpsRequest.readyState() == readyStateUnsent || httpsRequest.readyState() == readyStateDone)
  {
    requestOpenResult = httpsRequest.open(method, URL);

    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      httpsRequest.send();
      Serial.println("Request Sent");
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

void AsyncHTTPSWebRequest::sendDefaultHttpsRequest()
{
  sendHttpsRequest(defaultMethod, defaultURL);
}


void AsyncHTTPSWebRequest::init(AsyncHTTPSRequest::readyStateChangeCB cb, char *defURL, char *defMethod)
{
  httpsRequest.setDebug(false);
  httpsRequest.onReadyStateChange(cb);
  defaultURL = defURL;
  defaultMethod = defaultMethod;
}

String AsyncHTTPSWebRequest::response()
{
 newResponse = 0;
 return String(responseStr);
}

//--------------------------------------------

//HTTP

void AsyncHTTPWebRequest::httpRequestCB(void *optParm, AsyncHTTPRequest *request, int readyState)
{
  (void)optParm;

  if (readyState == readyStateDone)
  {
    if (request->responseHTTPcode() == 200)
    {
      responseStr = request->responseText();
      newResponse = 1;
    }

    request->setDebug(false);
  }
}

void AsyncHTTPWebRequest::sendHttpRequest(const char *URL, const char *method)
{
  static bool requestOpenResult;

  if (httpRequest.readyState() == readyStateUnsent || httpRequest.readyState() == readyStateDone)
  {
    requestOpenResult = httpRequest.open(method, URL);

    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      httpRequest.send();
      Serial.println("Request Sent");
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

void AsyncHTTPWebRequest::sendDefaultHttpRequest()
{
  sendHttpRequest(defaultMethod, defaultURL);
}


void AsyncHTTPWebRequest::init(AsyncHTTPRequest::readyStateChangeCB cb, char *defURL, char *defMethod)
{
  httpRequest.setDebug(false);
  httpRequest.onReadyStateChange(cb);
  defaultURL = defURL;
  defaultMethod = defaultMethod;
}

String AsyncHTTPWebRequest::response()
{
 newResponse = 0;
 return String(responseStr);
}
