#include "wled.h"

#define ESP_NOW_STATE_UNINIT       0
#define ESP_NOW_STATE_ON           1
#define ESP_NOW_STATE_ERROR        2


#define WLED_DISABLE_ESPNOW
#ifdef WLED_DISABLE_ESPNOW
void handleRemote(){}
#else

// stall change monitor struct

typedef struct msg_struct_stall_mon {
  bool toggleMode;
  bool stallChange;
} msg_struct_stall_mon;

// speed/stall ID monitor struct
typedef struct msg_struct_speed_stall_mon {
  float speed;
  int stallID;
} msg_struct_speed_stall_mon;

static int esp_now_state = ESP_NOW_STATE_UNINIT;
static msg_struct_stall_mon stall_mon_msg;
static msg_struct_speed_stall_mon speed_stall_mon_msg;

 
// Callback function that will be executed when data is received
#ifdef ESP8266
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif

  sprintf (last_signal_src, "%02x%02x%02x%02x%02x%02x",
    mac [0], mac [1], mac [2], mac [3], mac [4], mac [5]);

  if (strcmp(last_signal_src, linked_remote) != 0) {
    DEBUG_PRINT(F("ESP Now Message Received from Unlinked Sender: "));
    DEBUG_PRINTLN(last_signal_src);
    //return;
  }

  if (len == sizeof(stall_mon_msg))  {
   // handle stall change
  }

  else if (len == sizeof(speed_stall_mon_msg))  {
    // handle speed or stall
  }

  else  {
    DEBUG_PRINT(F("Unknown incoming ESP Now message received of length "));
    DEBUG_PRINTLN(len);
    return;
  }

  // memcpy(&(incoming.program), incomingData, sizeof(incoming));
  // uint32_t cur_seq = incoming.seq[0] | (incoming.seq[1] << 8) | (incoming.seq[2] << 16) | (incoming.seq[3] << 24);

  // if (cur_seq == last_seq) {
  //   return;
  // }

  
  DEBUG_PRINT(F("Incoming ESP Now Packet["));
  //DEBUG_PRINT(cur_seq);
  DEBUG_PRINT(F("] from sender["));
  DEBUG_PRINT(last_signal_src);
  DEBUG_PRINT(F("] button: "));
  //DEBUG_PRINTLN(incoming.button);

}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void handleRemote() {
  if (enable_espnow_remote) {
    if ((esp_now_state == ESP_NOW_STATE_UNINIT) && (interfacesInited || apActive)) { // ESPNOW requires Wifi to be initialized (either STA, or AP Mode) 
      DEBUG_PRINTLN(F("Initializing ESP_NOW listener"));
      // Init ESP-NOW
      if (esp_now_init() != 0) {
        DEBUG_PRINTLN(F("Error initializing ESP-NOW"));
        esp_now_state = ESP_NOW_STATE_ERROR;
      }

      #ifdef ESP8266
      esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
      #endif

      // Once ESPNow is successfully Init, we will register for Send CB to
      // get the status of Trasnmitted packet
      esp_now_register_send_cb(OnDataSent);

      esp_now_register_recv_cb(OnDataRecv);
      esp_now_state = ESP_NOW_STATE_ON;
    }
  } else {
    if (esp_now_state == ESP_NOW_STATE_ON) {
      DEBUG_PRINTLN(F("Disabling ESP-NOW Remote Listener"));
      if (esp_now_deinit() != 0) {
        DEBUG_PRINTLN(F("Error de-initializing ESP-NOW"));
      }
      esp_now_state = ESP_NOW_STATE_UNINIT;
    } else if (esp_now_state == ESP_NOW_STATE_ERROR) {
      //Clear any error states (allows retrying by cycling)
      esp_now_state = ESP_NOW_STATE_UNINIT;
    }
  }
}

#endif
