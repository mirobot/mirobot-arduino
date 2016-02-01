#ifdef AVR
#ifndef __MIROBOTAVR_CPP__
#define __MIROBOTAVR_CPP__

#include "Arduino.h"
#include "Mirobot.h"

HotStepper motor1(&PORTB, 0b00011101);
HotStepper motor2(&PORTD, 0b11110000);

CmdProcessor p;

// Pointer to the bootloader memory location
void* bl = (void *) 0x3c00;

Mirobot::Mirobot(){
  blocking = true;
  lastLedChange = millis();
  calibratingSlack = false;
  beepComplete = 0;
  version(2);
}

void Mirobot::setup(){
  HotStepper::setup(TIMER1INT);
  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  // Set up the collision sensor inputs and state
  pinMode(LEFT_COLLIDE_SENSOR, INPUT_PULLUP);
  pinMode(RIGHT_COLLIDE_SENSOR, INPUT_PULLUP);
  _collideState = NORMAL;
  setPenState(UP);
  // Set up the status LED
  pinMode(STATUS_LED, OUTPUT);
  initSettings();
}

void Mirobot::setup(Stream &s){
  setup()
  // We will be non-blocking so we can continue to process serial input
  blocking = false;
  // Set up the command processor
  p.setup(s, self());
}

void Mirobot::initSettings(){
  if(EEPROM.read(EEPROM_OFFSET) == MAGIC_BYTE_1 && EEPROM.read(EEPROM_OFFSET + 1) == MAGIC_BYTE_2){
    // We've previously written something valid to the EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++){
      *((char*)&settings + t) = EEPROM.read(EEPROM_OFFSET + 2 + t);
    }
  }else{
    settings.hwmajor = 0;
    settings.hwminor = 0;
    settings.slackCalibration = 14;
    settings.moveCalibration = 1.0f;
    settings.turnCalibration = 1.0f;
    saveSettings();
  }
}

void Mirobot::saveSettings(){
  EEPROM.write(EEPROM_OFFSET, MAGIC_BYTE_1);
  EEPROM.write(EEPROM_OFFSET + 1, MAGIC_BYTE_2);
  for (unsigned int t=0; t<sizeof(settings); t++){
    EEPROM.write(EEPROM_OFFSET + 2 + t, *((char*)&settings + t));
  }
}

void Mirobot::setHwVersion(char &version){
  char v[4];
  int i;
  int v_ptr = 0;
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
  settings.hwmajor = atoi(v);
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
  settings.hwminor = atoi(v);
  saveSettings();
}

void Mirobot::takeUpSlack(byte motor1Dir, byte motor2Dir){
  // Take up the slack on each motor
  if(motor1.lastDirection != motor1Dir){
    motor1.turn(settings.slackCalibration, motor1Dir);
  }
  if(motor2.lastDirection != motor2Dir){
    motor2.turn(settings.slackCalibration, motor2Dir);
  }
}

void Mirobot::forward(int distance){
  takeUpSlack(FORWARD, BACKWARD);
  motor1.turn(distance * steps_per_mm * settings.moveCalibration, FORWARD);
  motor2.turn(distance * steps_per_mm * settings.moveCalibration, BACKWARD);
  wait();
}

void Mirobot::back(int distance){
  takeUpSlack(BACKWARD, FORWARD);
  motor1.turn(distance * steps_per_mm * settings.moveCalibration, BACKWARD);
  motor2.turn(distance * steps_per_mm * settings.moveCalibration, FORWARD);
  wait();
}

void Mirobot::left(int angle){
  takeUpSlack(FORWARD, FORWARD);
  motor1.turn(angle * steps_per_degree * settings.turnCalibration, FORWARD);
  motor2.turn(angle * steps_per_degree * settings.turnCalibration, FORWARD);
  wait();
}

void Mirobot::right(int angle){
  takeUpSlack(BACKWARD, BACKWARD);
  motor1.turn(angle * steps_per_degree * settings.turnCalibration, BACKWARD);
  motor2.turn(angle * steps_per_degree * settings.turnCalibration, BACKWARD);
  wait();
}

void Mirobot::penup(){
  setPenState(UP);
  wait();
}

void Mirobot::pendown(){
  setPenState(DOWN);
  wait();
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
  calibratingSlack = false;
}

void Mirobot::reset(){
  // Give the response message time to get out
  delay(100);
  goto *bl;
}

void Mirobot::follow(){
  following = true;
}

int Mirobot::followState(){
  return analogRead(LEFT_LINE_SENSOR) - analogRead(RIGHT_LINE_SENSOR);
}

void Mirobot::collide(){
  colliding = true;
}

void Mirobot::collideState(char &state){
  boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
  boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
  if(collideLeft && collideRight){
    strcpy(&state, "both");
  }else if(collideLeft){
    strcpy(&state, "left");
  }else if(collideRight){
    strcpy(&state, "right");
  }else{
    strcpy(&state, "none");
  }
}

void Mirobot::beep(int duration){
  tone(SPEAKER_PIN, NOTE_C4, duration);
  beepComplete = millis() + duration;
}

boolean Mirobot::ready(){
  return (motor1.ready() && motor2.ready() && !servo_pulses_left && beepComplete < millis());
}

void Mirobot::wait(){
  if(blocking){
    while(!ready()){
      if(servo_pulses_left){
        servoHandler();
      }
    }
  }
}

void Mirobot::setPenState(penState_t state){
  penState = state;
  servo_pulses_left = SERVO_PULSES;
  next_servo_pulse = 0;
}

void Mirobot::followHandler(){
  if(motor1.ready() && motor2.ready()){
    int diff = analogRead(LEFT_LINE_SENSOR) - analogRead(RIGHT_LINE_SENSOR);
    if(diff > 5){
      if(versionNum == 1){
        right(1);
      }else if(versionNum == 2){
        left(1);
      }
    }else if(diff < -5){
      if(versionNum == 1){
        left(1);
      }else if(versionNum == 2){
        right(1);
      }
    }else{
      forward(5);
    }
  }
}

void Mirobot::collideHandler(){
  boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
  boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
  if(_collideState == NORMAL){
    if(collideLeft){
      _collideState = LEFT_REVERSE;
      back(50);
    }else if(collideRight){
      _collideState = RIGHT_REVERSE;
      back(50);
    }else{
      forward(10);
    }
  }else if(motor1.ready() && motor2.ready()){
    switch(_collideState){
      case LEFT_REVERSE :
        _collideState = LEFT_TURN;
        right(90);
        break;
      case RIGHT_REVERSE :
        _collideState = RIGHT_TURN;
        left(90);
        break;
      case LEFT_TURN :
      case RIGHT_TURN :
        _collideState = NORMAL;
      case NORMAL :
        break;
    }
  }
}

void Mirobot::ledHandler(){
  long t = millis();
  digitalWrite(STATUS_LED, (!((t / 100) % 10) || !(((t / 100) - 2) % 10)));
}

void Mirobot::servoHandler(){
  if(servo_pulses_left){
    if(micros() >= next_servo_pulse){
      servo_pulses_left--;
      digitalWrite(SERVO_PIN, HIGH);
      if(penState == UP){
        next_servo_pulse = micros() + (12000 - penup_delay);
        delayMicroseconds(penup_delay);
      }else{
        next_servo_pulse = micros() + (12000 - pendown_delay);
        delayMicroseconds(pendown_delay);
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

void Mirobot::sensorNotifier(){
  if(collideNotify){
    boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
    boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
    char currentCollideState = collideRight | (collideLeft << 1);
    if(currentCollideState != lastCollideState){
      if(collideLeft && collideRight){
        p.collideNotify("both");
      }else if(collideLeft){
        p.collideNotify("left");
      }else if(collideRight){
        p.collideNotify("right");
      }
      lastCollideState = currentCollideState;
    }
  }
  if(followNotify){
    int currentFollowState = analogRead(LEFT_LINE_SENSOR) - analogRead(RIGHT_LINE_SENSOR);
    if(currentFollowState != lastFollowState){
      p.followNotify(currentFollowState);
    }
    lastFollowState = currentFollowState;
  }
}

// This allows for runtime configuration of which hardware is used
void Mirobot::version(char v){
  versionNum = v;
  if(v == 1){
    steps_per_mm = STEPS_PER_MM_V1;
    steps_per_degree = STEPS_PER_DEGREE_V1;
    penup_delay = PENUP_DELAY_V1;
    pendown_delay = PENDOWN_DELAY_V1;
  }else if(v == 2 || v == 3){
    steps_per_mm = STEPS_PER_MM_V2;
    steps_per_degree = STEPS_PER_DEGREE_V2;
    penup_delay = PENUP_DELAY_V2;
    pendown_delay = PENDOWN_DELAY_V2;
  }
}

void Mirobot::calibrateSlack(unsigned int amount){
  settings.slackCalibration = amount;
  saveSettings();
  calibratingSlack = true;
  motor1.turn(1, FORWARD);
  motor2.turn(1, BACKWARD);
}

void Mirobot::calibrateMove(float amount){
  settings.moveCalibration = amount;
  saveSettings();
}

void Mirobot::calibrateTurn(float amount){
  settings.turnCalibration = amount;
  saveSettings();
}

void Mirobot::calibrateHandler(){
  if(calibratingSlack && motor1.ready() && motor2.ready()){
    takeUpSlack((motor1.lastDirection == FORWARD ? BACKWARD : FORWARD), (motor2.lastDirection == FORWARD ? BACKWARD : FORWARD));
  }
}

void Mirobot::process(){
  ledHandler();
  servoHandler();
  autoHandler();
  calibrateHandler();
  sensorNotifier();
  p.process();
}
#endif
#endif