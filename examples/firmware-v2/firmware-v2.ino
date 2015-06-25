#include <Mirobot.h>
#include <EEPROM.h>

Mirobot mirobot;

void setup(){
  Serial.begin(57600);
  mirobot.setup(Serial);
}

void loop(){
  mirobot.process();
}
