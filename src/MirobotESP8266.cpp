#ifdef ESP8266

#include "Arduino.h"
#include "Mirobot.h"

ShiftStepper motor1(0);
ShiftStepper motor2(1);

MirobotWifi wifi;

CmdManager manager;
static char tmpBuff[10];

Mirobot::Mirobot(){
  blocking = true;
  lastLedChange = millis();
  calibratingSlack = false;
  beepComplete = 0;
  version(3);
}

void Mirobot::begin(){
  ShiftStepper::setup(SHIFT_REG_DATA, SHIFT_REG_CLOCK, SHIFT_REG_LATCH);
  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  // Set up the collision sensor inputs and state
  //pinMode(LEFT_COLLIDE_SENSOR, INPUT_PULLUP);
  //pinMode(RIGHT_COLLIDE_SENSOR, INPUT_PULLUP);
  _collideStatus = NORMAL;
  setPenState(UP);
  // Set up the status LED
  //pinMode(STATUS_LED, OUTPUT);
  initSettings();
  initCmds();
}

void Mirobot::enableSerial(){
  blocking = false;
  // Set up Serial and add it to be processed
  Serial.begin(230400);
  Serial.setTimeout(1); 
  manager.addStream(Serial);
}

void Mirobot::enableWifi(){
  wifi.begin();
}

void Mirobot::initCmds(){
  manager.setMirobot(self());
  //             Command name        Handler function             // Returns immediately
  manager.addCmd("version",          &Mirobot::_version,          true);
  manager.addCmd("ping",             &Mirobot::_ping,             true);
  manager.addCmd("uptime",           &Mirobot::_uptime,           true);
  manager.addCmd("uptime",           &Mirobot::_uptime,           true);
  manager.addCmd("pause",            &Mirobot::_pause,            true);
  manager.addCmd("resume",           &Mirobot::_resume,           true);
  manager.addCmd("stop",             &Mirobot::_stop,             true);
  manager.addCmd("collideState",     &Mirobot::_collideState,     true);
  manager.addCmd("collideNotify",    &Mirobot::_collideNotify,    true);
  manager.addCmd("followState",      &Mirobot::_followState,      true);
  manager.addCmd("followNotify",     &Mirobot::_followNotify,     true);
  manager.addCmd("slackCalibration", &Mirobot::_slackCalibration, true);
  manager.addCmd("moveCalibration",  &Mirobot::_moveCalibration,  true);
  manager.addCmd("turnCalibration",  &Mirobot::_turnCalibration,  true);
  manager.addCmd("calibrateMove",    &Mirobot::_calibrateMove,    true);
  manager.addCmd("calibrateTurn",    &Mirobot::_calibrateTurn,    true);
  manager.addCmd("forward",          &Mirobot::_forward,          false);
  manager.addCmd("back",             &Mirobot::_back,             false);
  manager.addCmd("right",            &Mirobot::_right,            false);
  manager.addCmd("left",             &Mirobot::_left,             false);
  manager.addCmd("penup",            &Mirobot::_penup,            false);
  manager.addCmd("pendown",          &Mirobot::_pendown,          false);
  manager.addCmd("follow",           &Mirobot::_follow,           false);
  manager.addCmd("collide",          &Mirobot::_collide,          false);
  manager.addCmd("beep",             &Mirobot::_beep,             false);
  manager.addCmd("calibrateSlack",   &Mirobot::_calibrateSlack,   false);
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


void Mirobot::_version(char &arg, char &msg){
  strcpy(&msg, MIROBOT_VERSION);
}

void Mirobot::_ping(char &arg, char &msg){}

void Mirobot::_uptime(char &arg, char &msg){
  sprintf(&msg, "%lu", millis());
}

void Mirobot::_pause(char &arg, char &msg){
  pause();
}

void Mirobot::_resume(char &arg, char &msg){
  resume();
}

void Mirobot::_stop(char &arg, char &msg){
  stop();
}

void Mirobot::_collideState(char &arg, char &msg){
  collideState(msg);
}

void Mirobot::_collideNotify(char &arg, char &msg){
  collideNotify = !!strcmp(&arg, "false");
}

void Mirobot::_followState(char &arg, char &msg){
  sprintf(&msg, "%d", followState());
}

void Mirobot::_followNotify(char &arg, char &msg){
  followNotify = !!strcmp(&arg, "false");
}

void Mirobot::_slackCalibration(char &arg, char &msg){
  sprintf(&msg, "%d", settings.slackCalibration);
}

void Mirobot::_moveCalibration(char &arg, char &msg){
  dtostrf(settings.moveCalibration , 2, 6, &msg);
}

void Mirobot::_turnCalibration(char &arg, char &msg){
  dtostrf(settings.turnCalibration , 2, 6, &msg);
}

void Mirobot::_calibrateMove(char &arg, char &msg){
  calibrateMove(atof(&arg));
}

void Mirobot::_calibrateTurn(char &arg, char &msg){
  calibrateTurn(atof(&arg));
}

void Mirobot::_forward(char &arg, char &msg){
  forward(atoi(&arg));
}

void Mirobot::_back(char &arg, char &msg){
  back(atoi(&arg));
}

void Mirobot::_right(char &arg, char &msg){
  right(atoi(&arg));
}

void Mirobot::_left(char &arg, char &msg){
  left(atoi(&arg));
}

void Mirobot::_penup(char &arg, char &msg){
  penup();
}

void Mirobot::_pendown(char &arg, char &msg){
  pendown();
}

void Mirobot::_follow(char &arg, char &msg){
  follow();
}

void Mirobot::_collide(char &arg, char &msg){
  collide();
}

void Mirobot::_beep(char &arg, char &msg){
  beep(atoi(&arg));
}

void Mirobot::_calibrateSlack(char &arg, char &msg){
  calibrateSlack(atoi(&arg));
}

void Mirobot::saveSettings(){
  EEPROM.write(EEPROM_OFFSET, MAGIC_BYTE_1);
  EEPROM.write(EEPROM_OFFSET + 1, MAGIC_BYTE_2);
  for (unsigned int t=0; t<sizeof(settings); t++){
    EEPROM.write(EEPROM_OFFSET + 2 + t, *((char*)&settings + t));
  }
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
  /*
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
  */
  strcpy(&state, "none");
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
  /*
  boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
  boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
  if(_collideStatus == NORMAL){
    if(collideLeft){
      _collideStatus = LEFT_REVERSE;
      back(50);
    }else if(collideRight){
      _collideStatus = RIGHT_REVERSE;
      back(50);
    }else{
      forward(10);
    }
  }else if(motor1.ready() && motor2.ready()){
    switch(_collideStatus){
      case LEFT_REVERSE :
        _collideStatus = LEFT_TURN;
        right(90);
        break;
      case RIGHT_REVERSE :
        _collideStatus = RIGHT_TURN;
        left(90);
        break;
      case LEFT_TURN :
      case RIGHT_TURN :
        _collideStatus = NORMAL;
      case NORMAL :
        break;
    }
  }
  */
}

void Mirobot::ledHandler(){
  long t = millis();
  //digitalWrite(STATUS_LED, (!((t / 100) % 10) || !(((t / 100) - 2) % 10)));
  //digitalWrite(STATUS_LED, HIGH);
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
  /*
  if(collideNotify){
    boolean collideLeft = !digitalRead(LEFT_COLLIDE_SENSOR);
    boolean collideRight = !digitalRead(RIGHT_COLLIDE_SENSOR);
    char currentCollideState = collideRight | (collideLeft << 1);
    if(currentCollideState != lastCollideState){
      if(collideLeft && collideRight){
        manager.collideNotify("both");
      }else if(collideLeft){
        manager.collideNotify("left");
      }else if(collideRight){
        manager.collideNotify("right");
      }
      lastCollideState = currentCollideState;
    }
  }
  if(followNotify){
    int currentFollowState = analogRead(LEFT_LINE_SENSOR) - analogRead(RIGHT_LINE_SENSOR);
    if(currentFollowState != lastFollowState){
      manager.followNotify(currentFollowState);
    }
    lastFollowState = currentFollowState;
  }
  */
}

// This allows for runtime configuration of which hardware is used
void Mirobot::version(char v){
  versionNum = v;
  if(v == 3){
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

void Mirobot::checkReady(){
  if(manager.in_process && ready()){
    manager.sendComplete();
  }
}

void Mirobot::process(){
  ledHandler();
  servoHandler();
  autoHandler();
  calibrateHandler();
  sensorNotifier();
  checkReady();
  manager.process();
}

#endif