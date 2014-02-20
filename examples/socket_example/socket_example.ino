#include <stdlib.h>
#include <HotStepper.h>
#include <PWMServo.h>
#include <Mirobot.h>

Mirobot mirobot;

void setup(){
  Serial.begin(115200);
  mirobot.setup(Serial);
}

void loop(){
  mirobot.processInput();
}