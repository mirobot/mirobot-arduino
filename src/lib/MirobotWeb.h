#ifndef __MirobotWeb_h__
#define __MirobotWeb_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "lib/ESPAsyncTCP/ESPAsyncTCP.h"
#include "lib/ESPAsyncWebServer/ESPAsyncWebServer.h"

class MirobotWeb {
  public:
    MirobotWeb();
};

#endif
