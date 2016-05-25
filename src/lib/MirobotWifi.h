#ifndef __MirobotWifi_h__
#define __MirobotWifi_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "lib/MirobotWeb.h"
#include "lib/MirobotWebSocket.h"
#include "Mirobot.h"
#include <DNSServer.h>
#include <Ticker.h>
#include "lib/Discovery.h"
#include "lib/ArduinoJson/ArduinoJson.h"

typedef void (* dataHandler) (char *);

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
    static void startWifiScan();
    void setupWifi();
    static bool networkChanged;
    static bool wifiScanReady;
    static MirobotSettings * settings;
    void getWifiScanData(ArduinoJson::JsonArray &);
    void onMsg(dataHandler);
    static void sendWebSocketMsg(ArduinoJson::JsonObject &);
  private:
    bool enabled;
    static bool wifiScanRequested;
    MirobotWeb webServer;
    DNSServer dnsServer;
    void setupDNS();
    static void staCheck();
};

#endif
