#ifdef AVR

#include "Arduino.h"
#include "HotStepper.h"

HotStepper *HotStepper::firstInstance;

HotStepper::HotStepper(volatile uint8_t* port, byte pinmask) {
  _port = port;
  _pinmask = pinmask;
  _remaining = 0;
  _paused = false;
  counter = PULSE_PER_3MS;
  release();
  if(firstInstance){
    firstInstance->addNext(this);
  }else{
    firstInstance = this;
  }
}

void HotStepper::addNext(HotStepper *ref){
  if(nextInstance){
    nextInstance->addNext(ref);
  }else{
    nextInstance = ref;
  }
}

void HotStepper::instanceSetup(){
  if(_port == &PORTB){
    DDRB |= _pinmask;
  }else if(_port == &PORTC){
    DDRC |= _pinmask;
  }else if(_port == &PORTD){
    DDRD |= _pinmask;
  }
  if(nextInstance){
    nextInstance->instanceSetup();
  }
}

void HotStepper::begin(){
  if(firstInstance){
    firstInstance->instanceSetup();
  }
  disableInterrupts();
}

void HotStepper::pause(){
  _paused = true;
}

void HotStepper::resume(){
  _paused = false;
}

void HotStepper::stop(){
  _remaining = 0;
}

void HotStepper::turn(long steps, byte direction){
  turn(steps, direction, 1.0);
}

void HotStepper::turn(long steps, byte direction, float rate){
  if(steps < 0){
    steps = -steps;
    direction = !direction;
  }
  _remaining = steps;
  _dir = direction;
  _counterMax = PULSE_PER_3MS / rate;
  counter = _counterMax;
  lastDirection = direction;
  enableInterrupts();
}

boolean HotStepper::ready(){
  return (_remaining == 0);
}

long HotStepper::remaining(){
  return _remaining;
}

byte HotStepper::nextStep(){
  byte currentStep = unpad(((byte)*_port), _pinmask);
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

void HotStepper::setNextStep(){
  if(_remaining > 0 && !_paused){
    _remaining--;
    setStep(nextStep());
  }else{
    release();
  }
}

void HotStepper::setStep(byte state){
  byte nextState = (byte)*_port;
  nextState &= ~_pinmask;
  nextState |= pad(state, _pinmask);
  *_port = nextState;
}

byte HotStepper::pad(byte input, byte mask){
  byte output = 0;
  for(char i=0; i<8; i++){
    if((mask & 1)){
      //We should use this pin
      if(input &1){
        output |= 0b10000000;
      }else{
        output &= 0b01111111;
      }
      input >>= 1;
    }
    if(i<7){
      output >>= 1;
      mask >>= 1;
    }
  }
  return output;
}

byte HotStepper::unpad(byte input, byte mask){
  byte output = 0;
  for(char i=0; i<8; i++){
    if((mask & 1)){
      //We should use this pin
      if(input & 1){
        output |= 0b00001000;
      }else{
        output &= 0b11110111;
      }
      if(mask > 1){
        output >>= 1;
      }
    }
    input >>= 1;
    mask >>= 1;
  }
  return output;
}
void HotStepper::release(){
  setStep(0);
  if(firstInstance->checkReady()){
    disableInterrupts();
  }
}

boolean HotStepper::checkReady(){
  if(nextInstance){
    return ready() && nextInstance->checkReady();
  }else{
    return ready();
  }
}

void HotStepper::enableInterrupts(){
  if(TCCR1A == 0 && TCCR1B == 0){
    // initialize Timer1 for a 3ms duty cycle
    cli();        // disable global interrupts
    TCNT1  = 0;   // initialize counter value to 0
    // set compare match register to desired timer count:
    OCR1A = ((48000 / PULSE_PER_3MS)-1) / (16/clockCyclesPerMicrosecond());
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 bit for no prescaler:
    TCCR1B |= (1 << CS10);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    sei();        // enable global interrupts
  }
}

void HotStepper::disableInterrupts(){
  cli();        // disable global interrupts
  TCCR1A = 0;   // set entire TCCR1A register to 0
  TCCR1B = 0;   // same for TCCR1B
  sei();        // enable global interrupts
}

void HotStepper::trigger(){
  if(!--counter){
    counter = _counterMax;
    setNextStep();
  }
  if(nextInstance){
    nextInstance->trigger();
  }
}

void HotStepper::triggerTop(){
  if(firstInstance){
    firstInstance->trigger();
  }
}

ISR(TIMER1_COMPA_vect)
{
  HotStepper::triggerTop();
}

#endif //#ifdef AVR
