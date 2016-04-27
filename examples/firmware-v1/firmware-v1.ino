#include <Mirobot.h>

Mirobot mirobot;

void setup(){
  mirobot.begin(1);
  mirobot.enableSerial();
}

void loop(){
  mirobot.loop();
}
