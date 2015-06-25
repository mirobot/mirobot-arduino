#include <Mirobot.h>
#include <EEPROM.h>

/*
This sketch demonstrates simple collision detection without using the built in library
*/

Mirobot mirobot;
typedef enum {NORMAL, RIGHT_REVERSE, RIGHT_TURN, LEFT_REVERSE, LEFT_TURN} collideState_ty;
collideState_ty collideState;

void setup(){
  mirobot.setup();
}

void loop(){
  boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
  boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
  if(collideState == NORMAL){
    if(collideLeft){
      collideState = LEFT_REVERSE;
      mirobot.back(50);
    }else if(collideRight){
      collideState = RIGHT_REVERSE;
      mirobot.back(50);
    }else{
      mirobot.forward(10);
    }
  }else if(motor1.ready() && motor2.ready()){
    switch(collideState){
      case LEFT_REVERSE :
        collideState = LEFT_TURN;
        mirobot.right(90);
        break;
      case RIGHT_REVERSE :
        collideState = RIGHT_TURN;
        mirobot.left(90);
        break;
      case LEFT_TURN :
      case RIGHT_TURN :
        collideState = NORMAL;
    }
  }
}