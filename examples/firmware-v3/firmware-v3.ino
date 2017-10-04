#include <Mirobot.h>
#include "web.h"

Mirobot mirobot;

void setup(){
  mirobot.enableSerial();
  mirobot.enableWifi();
  mirobot.begin();
}

void loop(){
  mirobot.loop();
}

