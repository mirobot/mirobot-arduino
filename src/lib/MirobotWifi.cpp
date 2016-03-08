#ifdef ESP8266

#include "Arduino.h"
#include "MirobotWifi.h"

void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.print("WiFi connected - IP address: ");
            Serial.println(WiFi.localIP());
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("WiFi lost connection");
            break;
    }
}

MirobotWifi::MirobotWifi() {
  enabled = false;
}

void MirobotWifi::begin(){
  uint8_t mac[6];
  
  // Put the WiFi into AP_STA mode for 10 seconds
  WiFi.mode(WIFI_AP_STA);
  
  setupAP();
  setupSTA();

  webServer = MirobotWeb();

	enabled = true;
}

void MirobotWifi::setupAP(){
  uint8_t mac[6];

  // Set the hostname
  WiFi.softAPmacAddress(mac);
  sprintf(hostname, "Mirobot-%02X%02X", mac[4], mac[5]);
  WiFi.softAP(hostname);
}

void MirobotWifi::setupSTA(){
  WiFi.onEvent(WiFiEvent);
  WiFi.begin("***", "********");
}

void MirobotWifi::run(){
  if(!enabled) return;
  webServer.run();
}

#endif
