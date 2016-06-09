#include <Mirobot.h>

Mirobot mirobot;

void setup(){
  mirobot.begin();
  mirobot.enableSerial();
  mirobot.enableWifi();
}

void loop(){
  mirobot.loop();
}

