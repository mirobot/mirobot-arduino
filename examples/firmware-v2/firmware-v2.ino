#include <Mirobot.h>

Mirobot mirobot;

void setup(){
  mirobot.begin();
  mirobot.enableSerial();
}

void loop(){
  mirobot.loop();
}
