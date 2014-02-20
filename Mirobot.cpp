#include "Arduino.h"
#include "Mirobot.h"

HotStepper motor1(&PORTC, 0);
HotStepper motor2(&PORTD, 4);
PWMServo pen;
CmdProcessor::CmdProcessor p;

Mirobot::Mirobot(){
  blocking = true;
  mainState = POWERED_UP;
  lastLedChange = millis();
  pen.attach(SERVO_PIN_A);
}

void Mirobot::setup(Stream &s){
  HotStepper::setup();
  blocking = false;
  p.setup(s, self());
  // Set up the status LED
  pinMode(STATUS_LED, OUTPUT);
  // Set up the pins to communicate with the WiFi module
  pinMode(WIFI_READY, INPUT);  //nReady
  pinMode(WIFI_RESET, OUTPUT); // Reset
  digitalWrite(WIFI_RESET, HIGH); // Reset the wifi module when we reset
  delay(20);
  digitalWrite(WIFI_RESET, LOW); // Reset the wifi module when we reset
  delay(20);
  digitalWrite(WIFI_RESET, HIGH); // Reset the wifi module when we reset
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

void Mirobot::checkState(){
  if(!digitalRead(WIFI_READY)){
    mainState = CONNECTED;
  }else{
    mainState = POWERED_UP;
  }
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

void Mirobot::processInput(){
  ledHandler();
  p.process();
}