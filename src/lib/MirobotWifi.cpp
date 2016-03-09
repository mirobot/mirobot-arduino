#ifdef ESP8266

#include "Arduino.h"
#include "MirobotWifi.h"

Ticker tick;

bool MirobotWifi::networkChanged = false;

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      MirobotWifi::networkChanged = true;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      MirobotWifi::networkChanged = true;
      break;
  }
}

MirobotWifi::MirobotWifi() {
  enabled = false;
}

void MirobotWifi::begin(MirobotSettings * _settings){
  settings = _settings;

  // Subscribe to state change events
  WiFi.onEvent(WiFiEvent);
  
  setupWifi();
  setupDNS();

  webServer = MirobotWeb();

	enabled = true;
}

void MirobotWifi::defautAPName(char *name){
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  sprintf(name, "Mirobot-%02X%02X", mac[4], mac[5]);
}

IPAddress MirobotWifi::getStaIp(){
  return WiFi.localIP();
}

int32_t MirobotWifi::getStaRSSI(){
  return WiFi.RSSI();
}

WiFiMode MirobotWifi::getWifiMode(){
  return WiFi.getMode();
}

void MirobotWifi::setupWifi(){
  // Put the WiFi into AP_STA mode
  WiFi.mode(WIFI_AP_STA);

  // Reinitialise the WiFi
  WiFi.disconnect();

  // Set the hostname for DHCP
  WiFi.hostname(settings->ap_ssid);

  // Set up the access point
  if(strlen(settings->ap_pass)){
    WiFi.softAP(settings->ap_ssid, settings->ap_pass, settings->ap_channel);
  }else{
    WiFi.softAP(settings->ap_ssid, NULL, settings->ap_channel);
  }

  if(strlen(settings->sta_ssid)){
    // Configure the fixed IP if we're not using DHCP
    if(!settings->sta_dhcp && settings->sta_fixedip && settings->sta_fixedgateway && settings->sta_fixednetmask){
      WiFi.config(IPAddress(settings->sta_fixedip), IPAddress(settings->sta_fixedgateway), IPAddress(settings->sta_fixednetmask));
    }

    // Set up the STA connection
    WiFi.begin(settings->sta_ssid, settings->sta_pass);
    // Check if it's connected after 10 seconds
    tick.attach(10, MirobotWifi::staCheck);
  }else{
    WiFi.mode(WIFI_AP);
  }
}

void MirobotWifi::setupDNS(){
  dnsServer.start(53, "local.mirobot.io", IPAddress(192, 168, 4, 1));
}

void MirobotWifi::run(){
  if(!enabled) return;
  webServer.run();
  dnsServer.processNextRequest();
}

void MirobotWifi::staCheck(){
  tick.detach();
  if(!(uint32_t)WiFi.localIP()){
    WiFi.mode(WIFI_AP);
  }
}

#endif
