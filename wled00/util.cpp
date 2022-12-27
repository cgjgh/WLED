#include "wled.h"
#include "fcn_declare.h"
#include "const.h"


//helper to get int value at a position in string
int getNumVal(const String* req, uint16_t pos)
{
  return req->substring(pos+3).toInt();
}


// //helper to get int value with in/decrementing support via ~ syntax
// void parseNumber(const char* str, byte* val, byte minv, byte maxv)
// {
//   if (str == nullptr || str[0] == '\0') return;
//   if (str[0] == 'r') {*val = random8(minv,maxv?maxv:255); return;} // maxv for random cannot be 0
//   bool wrap = false;
//   if (str[0] == 'w' && strlen(str) > 1) {str++; wrap = true;}
//   if (str[0] == '~') {
//     int out = atoi(str +1);
//     if (out == 0) {
//       if (str[1] == '0') return;
//       if (str[1] == '-') {
//         *val = (int)(*val -1) < (int)minv ? maxv : min((int)maxv,(*val -1)); //-1, wrap around
//       } else {
//         *val = (int)(*val +1) > (int)maxv ? minv : max((int)minv,(*val +1)); //+1, wrap around
//       }
//     } else {
//       if (wrap && *val == maxv && out > 0) out = minv;
//       else if (wrap && *val == minv && out < 0) out = maxv;
//       else { 
//         out += *val;
//         if (out > maxv) out = maxv;
//         if (out < minv) out = minv;
//       }
//       *val = out;
//     }
//     return;
//   } else if (minv == maxv && minv == 0) { // limits "unset" i.e. both 0
//     byte p1 = atoi(str);
//     const char* str2 = strchr(str,'~'); // min/max range (for preset cycle, e.g. "1~5~")
//     if (str2) {
//       byte p2 = atoi(++str2);           // skip ~
//       if (p2 > 0) {
//         while (isdigit(*(++str2)));     // skip digits
//         parseNumber(str2, val, p1, p2);
//         return;
//       }
//     }
//   }
//   *val = atoi(str);
// }


// bool getVal(JsonVariant elem, byte* val, byte vmin, byte vmax) {
//   if (elem.is<int>()) {
// 		if (elem < 0) return false; //ignore e.g. {"ps":-1}
//     *val = elem;
//     return true;
//   } else if (elem.is<const char*>()) {
//     const char* str = elem;
//     size_t len = strnlen(str, 12);
//     if (len == 0 || len > 10) return false;
//     parseNumber(str, val, vmin, vmax);
//     return true;
//   }
//   return false; //key does not exist
// }


// bool updateVal(const char* req, const char* key, byte* val, byte minv, byte maxv)
// {
//   // const char *v = strstr(req, key);
//   // if (v) v += strlen(key);
//   // else return false;
//   // parseNumber(v, val, minv, maxv);
//   return true;
// // }


//append a numeric setting to string buffer
void sappend(char stype, const char* key, int val)
{
  char ds[] = "d.Sf.";

  switch(stype)
  {
    case 'c': //checkbox
      oappend(ds);
      oappend(key);
      oappend(".checked=");
      oappendi(val);
      oappend(";");
      break;
    case 'v': //numeric
      oappend(ds);
      oappend(key);
      oappend(".value=");
      oappendi(val);
      oappend(";");
      break;
    case 'i': //selectedIndex
      oappend(ds);
      oappend(key);
      oappend(SET_F(".selectedIndex="));
      oappendi(val);
      oappend(";");
      break;
  }
}


//append a string setting to buffer
void sappends(char stype, const char* key, char* val)
{
  switch(stype)
  {
    case 's': {//string (we can interpret val as char*)
      String buf = val;
      //convert "%" to "%%" to make EspAsyncWebServer happy
      //buf.replace("%","%%");
      oappend("d.Sf.");
      oappend(key);
      oappend(".value=\"");
      oappend(buf.c_str());
      oappend("\";");
      break;}
    case 'm': //message
      oappend(SET_F("d.getElementsByClassName"));
      oappend(key);
      oappend(SET_F(".innerHTML=\""));
      oappend(val);
      oappend("\";");
      break;
  }
}


bool oappendi(int i)
{
  char s[11];
  sprintf(s, "%d", i);
  return oappend(s);
}


bool oappend(const char* txt)
{
  uint16_t len = strlen(txt);
  if (olen + len >= SETTINGS_STACK_BUF_SIZE)
    return false;        // buffer full
  strcpy(obuf + olen, txt);
  olen += len;
  return true;
}


void prepareHostname(char* hostname)
{
  sprintf_P(hostname, "wled-%*s", 6, escapedMac.c_str() + 6);
  const char *pC = serverDescription;
  uint8_t pos = 5;          // keep "wled-"
  while (*pC && pos < 24) { // while !null and not over length
    if (isalnum(*pC)) {     // if the current char is alpha-numeric append it to the hostname
      hostname[pos] = *pC;
      pos++;
    } else if (*pC == ' ' || *pC == '_' || *pC == '-' || *pC == '+' || *pC == '!' || *pC == '?' || *pC == '*') {
      hostname[pos] = '-';
      pos++;
    }
    // else do nothing - no leading hyphens and do not include hyphens for all other characters.
    pC++;
  }
  //last character must not be hyphen
  if (pos > 5) {
    while (pos > 4 && hostname[pos -1] == '-') pos--;
    hostname[pos] = '\0'; // terminate string (leave at least "wled")
  }
}


bool isAsterisksOnly(const char* str, byte maxLen)
{
  for (byte i = 0; i < maxLen; i++) {
    if (str[i] == 0) break;
    if (str[i] != '*') return false;
  }
  //at this point the password contains asterisks only
  return (str[0] != 0); //false on empty string
}


//threading/network callback details: https://github.com/Aircoookie/WLED/pull/2336#discussion_r762276994
bool requestJSONBufferLock(uint8_t module)
{
  unsigned long now = millis();

  while (jsonBufferLock && millis()-now < 1000) delay(1); // wait for a second for buffer lock

  if (millis()-now >= 1000) {
    DEBUG_PRINT(F("ERROR: Locking JSON buffer failed! ("));
    DEBUG_PRINT(jsonBufferLock);
    DEBUG_PRINTLN(")");
    return false; // waiting time-outed
  }

  jsonBufferLock = module ? module : 255;
  DEBUG_PRINT(F("JSON buffer locked. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = &doc;  // used for applying presets (presets.cpp)
  doc.clear();
  return true;
}


void releaseJSONBufferLock()
{
  DEBUG_PRINT(F("JSON buffer released. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = nullptr;
  jsonBufferLock = 0;
}

