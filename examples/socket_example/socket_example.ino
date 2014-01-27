#include <stdlib.h>
#include <HotStepper.h>
#include <PWMServo.h>
#include <Mirobot.h>

Mirobot mirobot;

void setup(){
  mirobot.useRawSockets(Serial);
  mirobot.setup();
}

void loop(){
  mirobot.processInput();
}