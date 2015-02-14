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


/* This ARC2 implementation by arjen@openstem.com.au 2015-02-12
   Doing ARC2 because the turtle ends up where the pen finishes.

   With ARC, the turtle would stay in the original position
   (we could achieve his by doing a penup and reversing the motion)

   ARC2 angle radius
   Draw an arc with an included angle of a degrees and radius of r.
   However, the turtle ends up at the end of the arc.
   Example: ARC 45 100
   Ref: http://derrel.net/ep/logo/logo_com.htm#move

   Draw the arc in counterclockwise direction if radius is positive,
   otherwise in clockwise direction.
   Ref: https://docs.python.org/2/library/turtle.html#turtle.circle
*/
void Mirobot::arc2(int angle, int radius) {
  float inner_circle_radius, outer_circle_radius;
  float inner_circle_radians, outer_circle_radians;
  boolean clockwise, smallcircle;

  if (angle = 0)  // nothing to do
     return;
  angle = abs(angle) % 360; // make it clean rather than error

  // work out whether we're doing clockwise or anticlockwise arc
  clockwise = radius > 0 ? false : true;
  radius = abs(radius);

  /* Showing the working, then the shortcuts for the code!

     When drawing a circle with the pen, one turtle wheel will be walking
     a bigger (outer) circle and the other a smaller (inner) circle.

     Calculate radius of inner/outer circle using known wheel distance:
       inner_circle_radius = radius - (WHEEL_DISTANCE / 2.0)
       outer_circle_radius = radius + (WHEEL_DISTANCE / 2.0)

     Now calculate circumference of inner/outer circle: 2r * pi
       inner_circle_circumference = (2 * inner_circle_radius * 3.1416)
       outer_circle_circumference = (2 * outer_circle_radius * 3.1416)

     Now calculate distance required for angle given
       inner_circle_distance = (inner_circle_circumference / 360) * angle
       outer_circle_distance = (outer_circle_circumference / 360) * angle

     We can shortcut (2 * radius * pi) / 360, because
     2 * pi / 360 = pi / 180 = 0.017453333
     This is known as converting degrees to radians: radius / 0.017453333

     An arc of a circle with the same length as the radius of that circle
     corresponds to an angle of 1 radian. A full circle corresponds to an
     angle of 2π radians.
     Ref: https://en.wikipedia.org/wiki/Radian

     So we can say distance = (radius / 0.017453333) * angle
     and bypass the circumference bit in the middle of our calculation.
 */

  // Calculate radius of inner/outer circle using known wheel distance
  inner_circle_radius = radius - (WHEEL_DISTANCE / 2.0);
  outer_circle_radius = radius + (WHEEL_DISTANCE / 2.0);

  // Now calculate distance required for angle given
  inner_circle_radians = inner_circle_radius / 0.017453333;
  outer_circle_radians = outer_circle_radius / 0.017453333;

  // If circle radius is smaller than distance between pen and wheel,
  // we need to actually run the inner wheel backwards!
  if (inner_circle_distance < 0) {
    smallcircle = true;
    inner_circle_distance = abs(inner_circle_distance);
  }

  /* Now comes the real tricky bit. It's not the maths but the movement.

     The following code may change as we work out the best way of doing it.

     Ideally we'd make the speed adjustable per stepper, following
     the inner:outer distance ratio. That'll make it look good!

     For our now we'll just do the arc or circle in radian bits to "fake" it.
  */

  while (angle-- > 0) {
    // Note: motor1 on right, motor2 on left (reversed so backward moves forwards)
    if (clockwise) {
      motor1.turn(inner_circle_radians * STEPS_PER_MM, smallcircle ? BACKWARD : FORWARD);
      motor2.turn(outer_circle_radians * STEPS_PER_MM, BACKWARD);
    }
    else {
      motor1.turn(outer_circle_radians * STEPS_PER_MM, FORWARD);
      motor2.turn(inner_circle_radians * STEPS_PER_MM, smallcircle ? FORWARD : BACKWARD);
    }

    wait();
  }
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
}

boolean Mirobot::ready(){
  return (motor1.ready() && motor2.ready() && !servo_pulses_left);
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

void Mirobot::checkState(){
  if(!digitalRead(WIFI_READY)){
    mainState = CONNECTED;
  }else{
    mainState = POWERED_UP;
  }
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

void Mirobot::process(){
  ledHandler();
  servoHandler();
  autoHandler();
  sensorNotifier();
  p.process();
}
