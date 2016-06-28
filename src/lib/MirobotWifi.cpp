#ifdef ESP8266

#include "Arduino.h"
#include "MirobotWifi.h"

Ticker sta_tick;
Ticker discovery_tick;

bool MirobotWifi::networkChanged = false;
bool MirobotWifi::wifiScanRequested = false;
bool MirobotWifi::wifiScanReady = false;
MirobotSettings *MirobotWifi::settings;

void send_discovery(){
  send_discovery_request(WiFi.localIP(), MirobotWifi::settings->ap_ssid);
}

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      if(MirobotWifi::settings->discovery){
        send_discovery();
        discovery_tick.attach(300, send_discovery);
      }
      MirobotWifi::networkChanged = true;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      if(MirobotWifi::settings->discovery) discovery_tick.detach();
      MirobotWifi::networkChanged = true;
      break;
  }
}

MirobotWifi::MirobotWifi() {
  enabled = false;
  wifiScanRequested = false;
}

void MirobotWifi::begin(MirobotSettings * _settings){
  settings = _settings;

  // Subscribe to state change events
  WiFi.onEvent(WiFiEvent);
  
  setupWifi();
  setupDNS();

  // Start the web server
  webServer = MirobotWeb();

  // Start the WebSocket server
  beginWebSocket();

  enabled = true;
}

void MirobotWifi::onMsg(dataHandler h){
  setWsMsgHandler(h);
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
  // Don't let the ESP SDK persist the settings since we do this ourselves
  WiFi.persistent(false);

  // Put the WiFi into AP_STA mode
  WiFi.mode(WIFI_AP_STA);

  // Reinitialise the WiFi
  WiFi.disconnect();

  // Set the hostname for DHCP
  WiFi.hostname(settings->ap_ssid);

  // Set up the access point
  if(strlen(settings->ap_pass)){
    WiFi.softAP(settings->ap_ssid, settings->ap_pass);
  }else{
    WiFi.softAP(settings->ap_ssid);
  }

  if(strlen(settings->sta_ssid)){
    // Configure the fixed IP if we're not using DHCP
    if(!settings->sta_dhcp && settings->sta_fixedip && settings->sta_fixedgateway && settings->sta_fixednetmask){
      WiFi.config(IPAddress(settings->sta_fixedip), IPAddress(settings->sta_fixedgateway), IPAddress(settings->sta_fixednetmask));
    }

    // Set up the STA connection
    WiFi.begin(settings->sta_ssid, settings->sta_pass);
    // Check if it's connected after 10 seconds
    sta_tick.attach(10, MirobotWifi::staCheck);
  }else{
    WiFi.mode(WIFI_AP);
  }
}

void MirobotWifi::startWifiScan(){
  wifiScanRequested = true;
  WiFi.scanNetworks(true, true);
}

void MirobotWifi::getWifiScanData(ArduinoJson::JsonArray &msg){
  int count = WiFi.scanComplete();
  if(count < 0) return;
  for (int i = 0; i < count; ++i){
    JsonArray& net = msg.createNestedArray();
    net.add(WiFi.SSID(i));
    net.add(WiFi.encryptionType(i) != ENC_TYPE_NONE);
    net.add(WiFi.RSSI(i));
  }
}

void MirobotWifi::setupDNS(){
  dnsServer.start(53, "local.mirobot.io", IPAddress(192, 168, 4, 1));
}

void MirobotWifi::run(){
  if(!enabled) return;
  webServer.run();
  handleWebSocket();
  dnsServer.processNextRequest();
  if(wifiScanRequested && WiFi.scanComplete() >= 0){
    wifiScanRequested = false;
    wifiScanReady = true;
  }
}

void MirobotWifi::staCheck(){
  sta_tick.detach();
  if(!(uint32_t)WiFi.localIP()){
    WiFi.mode(WIFI_AP);
  }
}

void MirobotWifi::sendWebSocketMsg(ArduinoJson::JsonObject &outMsg){
  sendWsMsg(outMsg);
}

#endif
