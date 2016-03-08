#ifndef __MirobotWifi_h__
#define __MirobotWifi_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "lib/MirobotWeb.h"
#include <DNSServer.h>

class MirobotWifi {
  public:
    MirobotWifi();
    void begin();
    void run();
  private:
    bool enabled;
    char hostname[14];
    MirobotWeb webServer;
    DNSServer dnsServer;
    void setupAP();
    void setupSTA();
    void setupDNS();
};

#endif
