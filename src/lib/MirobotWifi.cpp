#ifdef ESP8266
#include "Arduino.h"
#include "MirobotWifi.h"

ESP8266WebServer server(80);

void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
}

MirobotWifi::MirobotWifi() {
}

void MirobotWifi::begin(){
  uint8_t mac[6];
  WiFi.printDiag(Serial);
  // Set the hostname
  WiFi.softAPmacAddress(mac);
  sprintf(hostname, "Mirobot-%02X%02X", mac[4], mac[5]);
  WiFi.softAP(hostname);
  
  // Put the WiFi into AP_STA mode for 10 seconds
  WiFi.mode(WIFI_AP_STA);
  
  server.on("/", handleRoot);
	server.begin();
}

#endif
