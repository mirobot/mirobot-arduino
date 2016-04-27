#include <Mirobot.h>

Mirobot mirobot;

void setup(){
  mirobot.begin();
  mirobot.version(1);
  mirobot.enableSerial();
}

void loop(){
  mirobot.loop();
}
