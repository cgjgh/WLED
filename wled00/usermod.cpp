#include "wled.h"

noDelay LEDtime2(150); // Creats a noDelay varible set to 1000ms

// gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

// gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
}

// loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
    if (cmd > 4)
    {
        if (cmd == 5)
        {
            digitalWrite(LED_BUILTIN, LOW);
        }
        else if (cmd == 6)
        {
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    else
    {
        if (blinkCounter < cmd)
        {
            if (LEDtime2.update()) // Checks to see if set time has past
            {
                // if the LED is off turn it on and vice-versa:
                if (ledState == LOW)
                {
                    ledState = HIGH;
                    blinkCounter++;
                }
                else
                    ledState = LOW;

                // set the LED with the ledState of the variable:
                digitalWrite(LED_BUILTIN, ledState);
            }
        }
        else
        {
            blinkCounter = 0;
            cmd = 0;
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
}

void forward()
{
    cmd = 1;
    DEBUG_PRINTLN(F("FORWARD"));
}

void reverse()
{
    cmd = 2;
    DEBUG_PRINTLN(F("REVERSE"));
}

void up()
{
    cmd = 3;
    DEBUG_PRINTLN(F("UP"));
}

void down()
{
    cmd = 4;
    DEBUG_PRINTLN(F("DOWN"));
}

void start()
{
    cmd = 5;
    DEBUG_PRINTLN(F("START"));
}
void end()
{
    cmd = 6;
    DEBUG_PRINTLN(F("END"));
}
