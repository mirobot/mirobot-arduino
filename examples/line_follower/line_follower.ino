#include <Mirobot.h>

/*
This sketch demonstrates simple line following without using the built in library
*/

Mirobot mirobot;

void setup(){
  mirobot.begin();
}

void loop(){
  int diff = analogRead(LEFT_LINE_SENSOR) - analogRead(RIGHT_LINE_SENSOR);
  if(diff > 5){
    mirobot.right(1);
  }else if(diff < -5){
    mirobot.left(1);
  }else{
    mirobot.forward(5);
  }
}