#include "Arduino.h"
#include "Mirobot.h"

HotStepper motor1(&PORTC, 0);
HotStepper motor2(&PORTD, 4);
PWMServo pen;
CmdProcessor::CmdProcessor p;

Mirobot::Mirobot(){
  blocking = true;
  pen.attach(SERVO_PIN_A);
}

void Mirobot::setup(){
  HotStepper::setup();
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
  pen.write(80);
}

void Mirobot::pendown(){
  pen.write(10);
}

boolean Mirobot::ready(){
  return (motor1.ready() && motor2.ready());
}

void Mirobot::setBlocking(boolean val){
  blocking = val;
}

void Mirobot::wait(){
  if(blocking){
    while(!ready()){}
  }
}

void Mirobot::setupCmdProcessor(char type, Stream &s){
  p.setup(s, self());
  p.socketType = type;
  blocking = false;
}

void Mirobot::useWebSockets(Stream &s){
  setupCmdProcessor(WEB_SOCKET, s);
}

void Mirobot::useRawSockets(Stream &s){
  setupCmdProcessor(RAW_SOCKET, s);
}

void Mirobot::processInput(){
  p.process();
}