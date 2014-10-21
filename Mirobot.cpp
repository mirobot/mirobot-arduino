#include "Arduino.h"
#include "Mirobot.h"

HotStepper motor1(&PORTB, 0b00011101);
HotStepper motor2(&PORTD, 0b11110000);
CmdProcessor::CmdProcessor p;

// Pointer to the bootloader memory location
void* bl = (void *) 0x3c00;

Mirobot::Mirobot(){
  blocking = true;
  mainState = POWERED_UP;
  lastLedChange = millis();
}

void Mirobot::setup(Stream &s){
  HotStepper::setup();
  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  setPenState(UP);
  // We will be non-blocking so we can continue to process serial input
  blocking = false;
  // Set up the command processor
  p.setup(s, self());
  // Set up the status LED
  pinMode(STATUS_LED, OUTPUT);
  // Set up the ready pin to communicate with the WiFi module
  pinMode(WIFI_READY, INPUT);  //nReady
  penup();
}

void Mirobot::forward(int distance){
  motor1.turn(distance * STEPS_PER_MM, FORWARD);
  motor2.turn(distance * STEPS_PER_MM, BACKWARD);
  wait();
}

void Mirobot::back(int distance){
  motor1.turn(distance * STEPS_PER_MM, BACKWARD);
  motor2.turn(distance * STEPS_PER_MM, FORWARD);
  wait();
}

void Mirobot::left(int angle){
  motor1.turn(angle * STEPS_PER_DEGREE, FORWARD);
  motor2.turn(angle * STEPS_PER_DEGREE, FORWARD);
  wait();
}

void Mirobot::right(int angle){
  motor1.turn(angle * STEPS_PER_DEGREE, BACKWARD);
  motor2.turn(angle * STEPS_PER_DEGREE, BACKWARD);
  wait();
}

void Mirobot::penup(){
  setPenState(UP);
}

void Mirobot::pendown(){
  setPenState(DOWN);
}

void Mirobot::pause(){
  motor1.pause();
  motor2.pause();
  paused = true;
}

void Mirobot::resume(){
  motor1.resume();
  motor2.resume();
  paused = false;
}

void Mirobot::stop(){
  motor1.stop();
  motor2.stop();
  following = false;
  colliding = false;
}

void Mirobot::reset(){
  // Give the response message time to get out
  delay(100);
  goto *bl;
}

void Mirobot::follow(){
  following = true;
}

void Mirobot::collide(){
  colliding = true;
}

void Mirobot::beep(int duration){
  tone(SPEAKER_PIN, NOTE_C4, duration);
}

boolean Mirobot::ready(){
  return (motor1.ready() && motor2.ready() && !servo_pulses_left);
}

void Mirobot::setBlocking(boolean val){
  blocking = val;
}

void Mirobot::wait(){
  if(blocking){
    while(!ready()){}
  }
}

void Mirobot::setPenState(penState_t state){
  penState = state;
  servo_pulses_left = SERVO_PULSES;
  next_servo_pulse = 0;
}

void Mirobot::checkState(){
  if(!digitalRead(WIFI_READY)){
    mainState = CONNECTED;
  }else{
    mainState = POWERED_UP;
  }
}

void Mirobot::followHandler(){

}

void Mirobot::collideHandler(){

}

void Mirobot::ledHandler(){
  checkState();
  switch(mainState){
    case POWERED_UP:
      if(millis() - lastLedChange > 250){
        lastLedChange = millis();
        digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
      }
      break;
    case CONNECTED:
      digitalWrite(STATUS_LED, HIGH);
      break;
  }
}

void Mirobot::servoHandler(){
  if(servo_pulses_left){
    if(micros() >= next_servo_pulse){
      servo_pulses_left--;
      digitalWrite(SERVO_PIN, HIGH);
      if(penState == UP){
        next_servo_pulse = micros() + 10800;
        delayMicroseconds(1200);
      }else{
        next_servo_pulse = micros() + 10000;
        delayMicroseconds(2000);
      }
      digitalWrite(SERVO_PIN, LOW);
    } 
  }
}

void Mirobot::autoHandler(){
  if(following){
    followHandler();
  }else if(colliding){
    collideHandler();
  }
}

void Mirobot::process(){
  ledHandler();
  servoHandler();
  autoHandler();
  p.process();
}