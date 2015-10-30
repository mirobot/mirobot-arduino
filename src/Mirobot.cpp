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
  slackCalibration = 14;
  moveCalibration = 1.0f;
  turnCalibration = 1.0f;
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
  //setPenState(UP);
  // Set up the status LED
  pinMode(STATUS_LED, OUTPUT);
}

void Mirobot::setup(Stream &s){
  HotStepper::setup(TIMER1INT);
  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  // Set up the collision sensor inputs and state
  pinMode(LEFT_COLLIDE_SENSOR, INPUT_PULLUP);
  pinMode(RIGHT_COLLIDE_SENSOR, INPUT_PULLUP);
  _collideState = NORMAL;
  setPenState(UP);
  // We will be non-blocking so we can continue to process serial input
  blocking = false;
  // Set up the command processor
  p.setup(s, self());
  // Set up the status LED
  pinMode(STATUS_LED, OUTPUT);
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

void Mirobot::takeUpSlack(byte motor1Dir, byte motor2Dir){
  // Take up the slack on each motor
  if(motor1.lastDirection != motor1Dir){
    motor1.turn(slackCalibration, motor1Dir);
  }
  if(motor2.lastDirection != motor2Dir){
    motor2.turn(slackCalibration, motor2Dir);
  }
  // Wait until the motors have taken up the slack
  while(!motor1.ready() && !motor2.ready()){}
}

void Mirobot::forward(int distance){
  takeUpSlack(FORWARD, BACKWARD);
  motor1.turn(distance * steps_per_mm * moveCalibration, FORWARD);
  motor2.turn(distance * steps_per_mm * moveCalibration, BACKWARD);
  wait();
}

void Mirobot::back(int distance){
  takeUpSlack(BACKWARD, FORWARD);
  motor1.turn(distance * steps_per_mm * moveCalibration, BACKWARD);
  motor2.turn(distance * steps_per_mm * moveCalibration, FORWARD);
  wait();
}

void Mirobot::left(int angle){
  takeUpSlack(FORWARD, FORWARD);
  motor1.turn(angle * steps_per_degree * turnCalibration, FORWARD);
  motor2.turn(angle * steps_per_degree * turnCalibration, FORWARD);
  wait();
}

void Mirobot::right(int angle){
  takeUpSlack(BACKWARD, BACKWARD);
  motor1.turn(angle * steps_per_degree * turnCalibration, BACKWARD);
  motor2.turn(angle * steps_per_degree * turnCalibration, BACKWARD);
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
  if(v == 1){
    steps_per_mm = STEPS_PER_MM_V1;
    steps_per_degree = STEPS_PER_DEGREE_V1;
    penup_delay = PENUP_DELAY_V1;
    pendown_delay = PENDOWN_DELAY_V1;
  }else if(v == 2){
    steps_per_mm = STEPS_PER_MM_V2;
    steps_per_degree = STEPS_PER_DEGREE_V2;
    penup_delay = PENUP_DELAY_V2;
    pendown_delay = PENDOWN_DELAY_V2;
  }
}

void Mirobot::calibrateSlack(int amount){
  slackCalibration = amount;
  calibratingSlack = true;
  motor1.turn(1, FORWARD);
  motor2.turn(1, BACKWARD);
}

void Mirobot::calibrateMove(float amount){
  moveCalibration = amount;
}

void Mirobot::calibrateTurn(float amount){
  turnCalibration = amount;
}

void Mirobot::calibrateHandler(){
  if(calibratingSlack){
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
