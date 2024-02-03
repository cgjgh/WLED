#include "wled.h"

/*
 * Adalight and TPM2 handler
 */


uint16_t currentBaud = 1152; //default baudrate 115200 (divided by 100)
bool continuousSendLED = false;
uint32_t lastUpdate = 0;

void updateBaudRate(uint32_t rate){
  uint16_t rate100 = rate/100;
  if (rate100 == currentBaud || rate100 < 96) return;
  currentBaud = rate100;

  if (!pinManager.isPinAllocated(hardwareTX) || pinManager.getPinOwner(hardwareTX) == PinOwner::DebugOut){
    Serial.print(F("Baud is now ")); Serial.println(rate);
  }

  Serial.flush();
  Serial.begin(rate);
}

void handleSerial()
{
  if (pinManager.isPinAllocated(hardwareRX)) return;
  if (!Serial) return;              // arduino docs: `if (Serial)` indicates whether or not the USB CDC serial connection is open. For all non-USB CDC ports, this will always return true
}
