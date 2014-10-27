#define HOTSTEPPER_TIMER1
#include <stdlib.h>
#include <HotStepper.h>
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
