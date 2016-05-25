#ifndef __MirobotWeb_h__
#define __MirobotWeb_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

class MirobotWeb {
  public:
    MirobotWeb();
    void run();
};

#endif
