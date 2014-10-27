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
  HotStepper::setup(TIMER1INT);
  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  // Set up the collision sensor inputs and state
  pinMode(LEFT_COLLIDE_SENSOR, INPUT_PULLUP);
  pinMode(RIGHT_COLLIDE_SENSOR, INPUT_PULLUP);
  collideState = NORMAL;
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
  initHwVersion();
}

void Mirobot::initHwVersion(){
  if(EEPROM.read(0) == MAGIC_BYTE_1 && EEPROM.read(1) == MAGIC_BYTE_2){
    // We've previously written something valid to the EEPROM
    hwVersion.major = EEPROM.read(2);
    hwVersion.minor = EEPROM.read(3);
  }else{
    hwVersion.major = 0;
    hwVersion.minor = 0;
  }
}

void Mirobot::setHwVersion(char &version){
  char v[4];
  char i;
  char v_ptr = 0;
  char *ptr = &version;
  for(i = 0; i < 9; i++){
    if(ptr[i] >= 0x30 && ptr[i] <= 0x39){
      v[v_ptr++] = ptr[i];
    }
    if(ptr[i] == '.'){
      v[v_ptr++] = '\0';
      break;
    }
  }
  hwVersion.major = atoi(v);
  v_ptr = 0;
  for(i = i; i < 9; i++){
    if(ptr[i] >= 0x30 && ptr[i] <= 0x39){
      v[v_ptr++] = ptr[i];
    }
    if(ptr[i] == '\0'){
      v[v_ptr++] = '\0';
      break;
    }
  }
  v[v_ptr] = '\0';
  hwVersion.minor = atoi(v);
  EEPROM.write(0, MAGIC_BYTE_1);
  EEPROM.write(1, MAGIC_BYTE_2);
  EEPROM.write(2, hwVersion.major);
  EEPROM.write(3, hwVersion.minor);
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
  if(motor1.ready() && motor2.ready()){
    int diff = analogRead (LEFT_LINE_SENSOR) - analogRead (RIGHT_LINE_SENSOR);
    if(diff > 5){
      right(1);
    }else if(diff < -5){
      left(1);
    }else{
      forward(5);
    }
  }
}

void Mirobot::collideHandler(){
  boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
  boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
  if(collideState == NORMAL){
    if(collideLeft){
      Serial.println('l');
      collideState = LEFT_REVERSE;
      back(50);
    }else if(collideRight){
      Serial.println('r');
      collideState = RIGHT_REVERSE;
      back(50);
    }else{
      forward(10);
    }
  }else if(motor1.ready() && motor2.ready()){
    switch(collideState){
      case LEFT_REVERSE :
        collideState = LEFT_TURN;
        right(90);
        break;
      case RIGHT_REVERSE :
        collideState = RIGHT_TURN;
        left(90);
        break;
      case LEFT_TURN :
      case RIGHT_TURN :
        collideState = NORMAL;
    }
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