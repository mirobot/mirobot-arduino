#ifndef __MirobotWifi_h__
#define __MirobotWifi_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "lib/MirobotWeb.h"
#include "MirobotESP8266.h"
#include <DNSServer.h>
#include <Ticker.h>
#include "lib/Discovery.h"

struct MirobotSettings;

class MirobotWifi {
  public:
    MirobotWifi();
    void begin(MirobotSettings *);
    void run();
    static void defautAPName(char*);
    static IPAddress getStaIp();
    static int32_t getStaRSSI();
    static WiFiMode getWifiMode();
    void setupWifi();
    static bool networkChanged;
    static MirobotSettings * settings;
  private:
    bool enabled;
    MirobotWeb webServer;
    DNSServer dnsServer;
    void setupDNS();
    static void staCheck();
};

#endif
