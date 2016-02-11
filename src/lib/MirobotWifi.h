#ifndef __MirobotWifi_h__
#define __MirobotWifi_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>

class MirobotWifi {
  public:
    MirobotWifi();
    void begin();
  private:
    char hostname[14];
};

#endif
