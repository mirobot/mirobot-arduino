#include <Mirobot.h>
#include <EEPROM.h>

Mirobot mirobot;

void setup(){
  mirobot.begin();
  mirobot.enableSerial();
  mirobot.enableWifi();
}

void loop(){
  mirobot.process();
}
