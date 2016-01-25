// Version 72503c42 from github.com/bjpirt/ShiftStep

#define FROM_LIB
#include "Arduino.h"
#include "ShiftStep.h"

ShiftStep::ShiftStep(int data_pin, int clock_pin, int latch_pin) {
  _remaining = 0;
  _paused = false;
  release();
}

void ShiftStep::setup(){

}

void ShiftStep::pause(){
  _paused = true;
}

void ShiftStep::resume(){
  _paused = false;
}

void ShiftStep::stop(){
  _remaining = 0;
}

void ShiftStep::turn(long steps, byte direction){
  _remaining = steps;
  _dir = direction;
  lastDirection = direction;
}

boolean ShiftStep::ready(){
  return (_remaining == 0);
}

long ShiftStep::remaining(){
  return _remaining;
}

byte ShiftStep::nextStep(){
  byte currentStep = 0;//unpad(((byte)*_port), _pinmask);
  switch(currentStep){
    case B0000:
    case B0001:
      return (_dir == FORWARD ? B0010 : B1000);
    case B0010:
      return (_dir == FORWARD ? B0100 : B0001);
    case B0100:
      return (_dir == FORWARD ? B1000 : B0010);
    case B1000:
      return (_dir == FORWARD ? B0001 : B0100);
  }
}

void ShiftStep::setNextStep(){
  if(_remaining > 0 && !_paused){
    _remaining--;
    setStep(nextStep());
  }else{
    release();
  }
}

void ShiftStep::setStep(byte state){
}

void ShiftStep::release(){
  setStep(0);
}