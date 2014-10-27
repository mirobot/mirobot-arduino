#define HOTSTEPPER_TIMER1
#include <stdlib.h>
#include <HotStepper.h>
#include <Mirobot.h>
#include <EEPROM.h>

Mirobot mirobot;

/*
This sketch shows how you can program Mirobot directly in the Arduino environment.
*/

void setup(){
  mirobot.setup();
}

void loop(){
  //draw a square
  mirobot.pendown();
  for(char i=0; i<4; i++){
    mirobot.forward(100);
    mirobot.right(90);
  }
  mirobot.penup();
  mirobot.forward(150);

  //draw a star
  mirobot.pendown();
  for(char i=0; i<5; i++){
    mirobot.forward(100);
    mirobot.right(144);
  }
  mirobot.penup();

  delay(10000);
}
