#include <Mirobot.h>
#include <EEPROM.h>

Mirobot mirobot;

void setup(){
  mirobot.setup();
  mirobot.setupSerial();
  mirobot.setupWifi();
}

void loop(){
  mirobot.process();
}
