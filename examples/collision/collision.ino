#include <Mirobot.h>

/*
This sketch demonstrates simple collision detection without using the built in library
*/

Mirobot mirobot;
typedef enum {M_NORMAL, M_RIGHT_REVERSE, M_RIGHT_TURN, M_LEFT_REVERSE, M_LEFT_TURN} collideState_ty;
collideState_ty collideState;

void setup(){
  mirobot.begin();
}

void loop(){
  boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
  boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
  if(collideState == M_NORMAL){
    if(collideLeft){
      collideState = M_LEFT_REVERSE;
      mirobot.back(50);
    }else if(collideRight){
      collideState = M_RIGHT_REVERSE;
      mirobot.back(50);
    }else{
      mirobot.forward(10);
    }
  }else if(mirobot.ready()){
    switch(collideState){
      case M_LEFT_REVERSE :
        collideState = M_LEFT_TURN;
        mirobot.right(90);
        break;
      case M_RIGHT_REVERSE :
        collideState = M_RIGHT_TURN;
        mirobot.left(90);
        break;
      case M_LEFT_TURN :
      case M_RIGHT_TURN :
        collideState = M_NORMAL;
    }
  }
}