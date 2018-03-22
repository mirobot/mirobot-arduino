#ifdef ESP8266
#include "Arduino.h"
#include "ShiftStepper.h"

ShiftStepper *ShiftStepper::firstInstance;
int ShiftStepper::data_pin;
int ShiftStepper::clock_pin;
int ShiftStepper::latch_pin;
uint8_t ShiftStepper::lastBits;
uint8_t ShiftStepper::currentBits;

ShiftStepper::ShiftStepper(int offset) {
  _remaining = 0;
  _paused = false;
  motor_offset = offset;
  currentStep = 0;
  microCounter = UCOUNTER_DEFAULT;
  release();
  if(firstInstance){
    firstInstance->addNext(this);
  }else{
    firstInstance = this;
  }
}

void ShiftStepper::setup(int _data_pin, int _clock_pin, int _latch_pin){
  data_pin  = _data_pin;
  clock_pin = _clock_pin;
  latch_pin = _latch_pin;
  pinMode(data_pin,  OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(latch_pin, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(data_pin,  LOW);
  digitalWrite(clock_pin, LOW);
  digitalWrite(latch_pin, LOW);
  lastBits = 0;
  currentBits = 0;
  if(firstInstance){
    firstInstance->instanceSetup();
  }
}

void ShiftStepper::instanceSetup(){
  currentStep = 0;
  if(nextInstance){
    nextInstance->instanceSetup();
  }
}

void ShiftStepper::addNext(ShiftStepper *ref){
  if(nextInstance){
    nextInstance->addNext(ref);
  }else{
    nextInstance = ref;
  }
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
  if(steps < 0){
    steps = -steps;
    direction = !direction;
  }
  _remaining = steps;
  _dir = direction;
  lastDirection = direction;
  startTimer();
}

void ShiftStepper::turn(long steps, byte direction, float rate){
  turn(steps, direction);
}

boolean ShiftStepper::ready(){
  return (_remaining == 0);
}

long ShiftStepper::remaining(){
  return _remaining;
}

byte ICACHE_RAM_ATTR ShiftStepper::nextStep(){
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
    default:
      return B0000;
  }
}

void ICACHE_RAM_ATTR ShiftStepper::setNextStep(){
  if(_remaining > 0 && !_paused){
    if(!--microCounter){
      microCounter = UCOUNTER_DEFAULT;
      _remaining--;
      updateBits(nextStep());
    }
  }else{
    release();
    stopTimer();
  }
}

void ICACHE_RAM_ATTR ShiftStepper::release(){
  currentStep = 0;
  updateBits(0);
  sendBits();
}

void ICACHE_RAM_ATTR ShiftStepper::trigger(){
  setNextStep();
  if(nextInstance){
    nextInstance->trigger();
  }
}

void ICACHE_RAM_ATTR ShiftStepper::updateBits(uint8_t bits){
  currentStep = bits;
  bits &= B1111;
  bits <<= (motor_offset*4);
  currentBits &= ~(B1111 << (motor_offset *4));
  currentBits |= bits;
}

void ICACHE_RAM_ATTR ShiftStepper::sendBits(){
  if(currentBits != lastBits){
    lastBits = currentBits;
    shiftOut(data_pin, clock_pin, MSBFIRST, currentBits);
    digitalWrite(latch_pin, HIGH);
    digitalWrite(data_pin,  LOW);
    digitalWrite(clock_pin, LOW);
    digitalWrite(latch_pin, LOW);
  }
}

void ICACHE_RAM_ATTR ShiftStepper::triggerTop(){
  if(firstInstance){
    firstInstance->trigger();
  }
  sendBits();
}

void ShiftStepper::startTimer(){
  // Initialise the timers
  timer1_disable();
  timer1_isr_init();
  timer1_attachInterrupt(ShiftStepper::triggerTop);
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
  timer1_write(clockCyclesPerMicrosecond() * BASE_INTERRUPT_US);
}

void ShiftStepper::stopTimer(){
  timer1_disable();
}

#endif
