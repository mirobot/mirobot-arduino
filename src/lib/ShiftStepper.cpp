#ifdef ESP8266
#include "Arduino.h"
#include "ShiftStepper.h"

ShiftStepper *ShiftStepper::firstInstance;
int ShiftStepper::data_pin;
int ShiftStepper::clock_pin;
int ShiftStepper::latch_pin;

ShiftStepper::ShiftStepper(int motor_offset) {
  _remaining = 0;
  _paused = false;
  currentStep = 0;
  release();
  if(firstInstance){
    firstInstance->addNext(this);
  }else{
    firstInstance = this;
  }
}

void ShiftStepper::addNext(ShiftStepper *ref){
  if(nextInstance){
    nextInstance->addNext(ref);
  }else{
    nextInstance = ref;
  }
}

void ShiftStepper::instanceSetup(){
  if(nextInstance){
    nextInstance->instanceSetup();
  }
}

void ShiftStepper::setup(int _data_pin, int _clock_pin, int _latch_pin){
  data_pin = _data_pin;
  clock_pin = _clock_pin;
  latch_pin = _latch_pin;
  if(firstInstance){
    firstInstance->instanceSetup();
  }
  // Initialise the timers
}

void ShiftStepper::pause(){
  _paused = true;
}

void ShiftStepper::resume(){
  _paused = false;
}

void ShiftStepper::stop(){
  _remaining = 0;
}

void ShiftStepper::turn(long steps, byte direction){
  _remaining = steps;
  _dir = direction;
  lastDirection = direction;
}

boolean ShiftStepper::ready(){
  return (_remaining == 0);
}

long ShiftStepper::remaining(){
  return _remaining;
}

byte ShiftStepper::nextStep(){
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

void ShiftStepper::setNextStep(){
  if(_remaining > 0 && !_paused){
    _remaining--;
    setStep(nextStep());
  }else{
    release();
  }
}

void ShiftStepper::setStep(byte state){
  //Send to the shift register
}

void ShiftStepper::release(){
  setStep(0);
}

void ShiftStepper::trigger(){
  setNextStep();
  if(nextInstance){
    nextInstance->trigger();
  }
}

void ShiftStepper::triggerTop(){
  if(firstInstance){
    firstInstance->trigger();
  }
}

#endif
