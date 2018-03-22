#include "Mirobot.h"

Marceau<26> marcel;

Mirobot *Mirobot::mainInstance;

#ifdef AVR
// Set up the steppers
HotStepper rightMotor(&PORTB, 0b00011101);
HotStepper leftMotor(&PORTD, 0b11110000);
#endif //AVR

#ifdef ESP8266
static Ticker tick;
// Set up the steppers
ShiftStepper rightMotor(0);
ShiftStepper leftMotor(1);
#endif //ESP8266

Mirobot::Mirobot(){
  mainInstance = this;
  blocking = true;
  nextADCRead = 0;
  beepComplete = 0;
  initCmds();
}

void Mirobot::begin(unsigned char v){
  version(v);
  
#ifdef AVR
  // Initialise the steppers
  HotStepper::begin();
  // Set up the collision sensor inputs and state
  pinMode(LEFT_COLLIDE_SENSOR, INPUT_PULLUP);
  pinMode(RIGHT_COLLIDE_SENSOR, INPUT_PULLUP);
  // Set up the status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
#endif //AVR

#ifdef ESP8266
  // Set up the status LED pin
  pinMode(STATUS_LED_PIN, OUTPUT);
  // Set up the ADC on I2C
  Wire.begin(I2C_DATA, I2C_CLOCK);
  // Initialise the steppers
  ShiftStepper::setup(SHIFT_REG_DATA, SHIFT_REG_CLOCK, SHIFT_REG_LATCH);
  // Set up the line follower LED enable pin and turn it off
  pinMode(LINE_LED_ENABLE, OUTPUT);
  digitalWrite(LINE_LED_ENABLE, HIGH);
  // Set up the EEPROM
  EEPROM.begin(sizeof(MarceauSettings) + sizeof(settings)+4);
#endif //ESP8266

  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  _collideStatus = NORMAL;
  // Initialise the pen arm into the up position
  setPenState(UP);
  // Start the serial
  if(serialEnabled) beginSerial();
  // Start the WiFi
#ifdef ESP8266
  if(wifiEnabled) beginWifi();
#endif
  // Pull the settings out of memory
  initSettings();
  // Start marceau processing commands
  marcel.begin();
}

void Mirobot::begin(){
  begin(3);
}

void Mirobot::enableSerial(){
  // Enable serial processing
  serialEnabled = true;
}

void Mirobot::beginSerial(){
  // Use non-blocking mode to process serial
  blocking = false;
  // Set up Serial
  if(hwVersion == 1){
    Serial.begin(57600);
  }else if(hwVersion == 2){
    Serial.begin(57600);
  }else if(hwVersion == 3){
    Serial.begin(230400);
  }
  marcel.enableSerial(Serial);
  //Send out a boot message
  StaticJsonBuffer<60> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  outMsg["msg"] = versionStr;
  marcel.notify("boot", outMsg);
}

#ifdef ESP8266
void Mirobot::generateAPName(char * name){
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  sprintf(name, "Mirobot-%02X%02X", mac[4], mac[5]);
}

void Mirobot::enableWifi(){
  // Enable WiFi
  wifiEnabled = true;
}

void Mirobot::beginWifi(){
  static char defaultAPName[11];
  generateAPName(defaultAPName);
  marcel.setDefaultAPName(defaultAPName);
  marcel.setHostname("local.mirobot.io");
  marcel.enableWifi();
}

void Mirobot::sendDiscovery(){
  if(nextDiscovery < millis()){
    if(marcel.wifi.online){
      send_discovery_request((uint32_t)WiFi.localIP(), marcel.settings.ap_ssid, "Mirobot-v3");
      nextDiscovery = millis() + 30000;
    }else{
      nextDiscovery = millis() + 1000;
    }
  }
}
#endif //ESP8266

void Mirobot::initSettings(){
  uint16_t eepromOffset = sizeof(MarceauSettings) + 2;
  EEPROM.begin(sizeof(MarceauSettings) + sizeof(settings)+4);
  if(EEPROM.read(eepromOffset) == MAGIC_BYTE_1 && EEPROM.read(eepromOffset + 1) == MAGIC_BYTE_2 && EEPROM.read(eepromOffset + 2) == SETTINGS_VERSION){
    // We've previously written something valid to the EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++){
      *((char*)&settings + t) = EEPROM.read(eepromOffset + 2 + t);
    }
    // Sanity check the values to make sure they look correct
    if(settings.settingsVersion == SETTINGS_VERSION &&
       settings.slackCalibration < 50 &&
       abs(settings.moveCalibration) > 0.5f &&
       abs(settings.moveCalibration) < 1.5f &&
       abs(settings.turnCalibration) > 0.5f &&
       abs(settings.turnCalibration) < 1.5f){
      // The values look OK so let's leave them as they are
      return;
    }
  }
  // Either this is the first boot or the settings are bad so let's reset them
  settings.settingsVersion = SETTINGS_VERSION;
  settings.slackCalibration = 14;
  settings.moveCalibration = 1.0f;
  settings.turnCalibration = 1.0f;
#ifdef ESP8266
  settings.discovery = true;
#endif //ESP8266
  saveSettings();
}

void Mirobot::saveSettings(){
  EEPROM.begin(sizeof(MarceauSettings) + sizeof(settings)+4);
  uint16_t eepromOffset = sizeof(MarceauSettings) + 2;
  EEPROM.write(eepromOffset, MAGIC_BYTE_1);
  EEPROM.write(eepromOffset + 1, MAGIC_BYTE_2);
  for (unsigned int t=0; t<sizeof(settings); t++){
    EEPROM.write(eepromOffset + 2 + t, *((char*)&settings + t));
  }
#ifdef ESP8266
  EEPROM.commit();
#endif //ESP8266
}

void Mirobot::initCmds(){
  //                  Command name        Handler function   Returns immediately?
  marcel.addCmd("version",          _version,          true);
  marcel.addCmd("pause",            _pause,            true);
  marcel.addCmd("resume",           _resume,           true);
  marcel.addCmd("stop",             _stop,             true);
  marcel.addCmd("collideState",     _collideState,     true);
  marcel.addCmd("collideNotify",    _collideNotify,    true);
  marcel.addCmd("followState",      _followState,      true);
  marcel.addCmd("followNotify",     _followNotify,     true);
  marcel.addCmd("slackCalibration", _slackCalibration, true);
  marcel.addCmd("moveCalibration",  _moveCalibration,  true);
  marcel.addCmd("turnCalibration",  _turnCalibration,  true);
  marcel.addCmd("calibrateMove",    _calibrateMove,    true);
  marcel.addCmd("calibrateTurn",    _calibrateTurn,    true);
  marcel.addCmd("forward",          _forward,          false);
  marcel.addCmd("back",             _back,             false);
  marcel.addCmd("right",            _right,            false);
  marcel.addCmd("left",             _left,             false);
  marcel.addCmd("penup",            _penup,            false);
  marcel.addCmd("pendown",          _pendown,          false);
  marcel.addCmd("follow",           _follow,           false);
  marcel.addCmd("collide",          _collide,          false);
  marcel.addCmd("beep",             _beep,             false);
  marcel.addCmd("calibrateSlack",   _calibrateSlack,   false);
  marcel.addCmd("arc",              _arc,              false);
#ifdef ESP8266
  marcel.addCmd("updateFirmware",   _updateFirmware,   true);
  marcel.addCmd("updateUI",         _updateUI,         true);
#endif //ESP8266
}

void Mirobot::takeUpSlack(byte rightMotorDir, byte leftMotorDir){
  // Take up the slack on each motor
  if(rightMotor.lastDirection != rightMotorDir){
    rightMotor.turn(settings.slackCalibration, rightMotorDir);
  }
  if(leftMotor.lastDirection != leftMotorDir){
    leftMotor.turn(settings.slackCalibration, leftMotorDir);
  }
}

void Mirobot::forward(int distance){
  takeUpSlack(FORWARD, BACKWARD);
  rightMotor.turn(distance * steps_per_mm * settings.moveCalibration, FORWARD);
  leftMotor.turn(distance * steps_per_mm * settings.moveCalibration, BACKWARD);
  wait();
}

void Mirobot::back(int distance){
  takeUpSlack(BACKWARD, FORWARD);
  rightMotor.turn(distance * steps_per_mm * settings.moveCalibration, BACKWARD);
  leftMotor.turn(distance * steps_per_mm * settings.moveCalibration, FORWARD);
  wait();
}

void Mirobot::left(int angle){
  takeUpSlack(FORWARD, FORWARD);
  rightMotor.turn(angle * steps_per_degree * settings.moveCalibration * settings.turnCalibration, FORWARD);
  leftMotor.turn(angle * steps_per_degree * settings.moveCalibration * settings.turnCalibration, FORWARD);
  wait();
}

void Mirobot::right(int angle){
  takeUpSlack(BACKWARD, BACKWARD);
  rightMotor.turn(angle * steps_per_degree * settings.moveCalibration * settings.turnCalibration, BACKWARD);
  leftMotor.turn(angle * steps_per_degree * settings.moveCalibration * settings.turnCalibration, BACKWARD);
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
  rightMotor.pause();
  leftMotor.pause();
  paused = true;
}

void Mirobot::resume(){
  rightMotor.resume();
  leftMotor.resume();
  paused = false;
}

void Mirobot::stop(){
  rightMotor.stop();
  leftMotor.stop();
  following = false;
  colliding = false;
  calibratingSlack = false;
}

void Mirobot::follow(){
  following = true;
}

int Mirobot::followState(){
  return leftLineSensor - rightLineSensor;
}

void Mirobot::collide(){
  colliding = true;
}

collideState_t Mirobot::collideState(){
  readSensors();
  if(leftCollide && rightCollide){
    return BOTH;
  }else if(leftCollide){
    return LEFT;
  }else if(rightCollide){
    return RIGHT;
  }else{
    return NONE;
  }
}

void Mirobot::beep(int duration){
  tone(SPEAKER_PIN, NOTE_C4, duration);
  beepComplete = millis() + duration;
}

void Mirobot::arc(float angle, float radius){
  // Drawing an arc means drawing three concentric circular arcs with two wheels and a pen at the centre
  // So we need to work out the length of the outer, wheel arcs and then move them by that amount in the same time
  // To calculate the distance we can work out:
  //   circumference = 2 * pi * radius
  //   distance = circumference * (angle / 360)
  // combined:
  //   distance = 2 * pi * radius * (angle / 360)
  //   distance = 2 * 3.141593 * radius * (angle / 360)
  //   distance = 6.283185 * radius * (angle / 360)
  //   distance = 0.017453 * radius * angle
  float right_distance, left_distance;
  float right_rate, left_rate;
  int wheel_distance = 120;

  // extract the sign of the direction (+1 / -1) which will give us the correct distance to turn the steppers
  char dir = (radius > 0) - (radius < 0);

  // work out the distances each wheel has to move
  right_distance = 0.017453 * (radius - (wheel_distance / 2.0)) * angle * dir;
  left_distance = 0.017453 * (radius + (wheel_distance / 2.0)) * angle * dir;

  // work out the rate the wheel should move relative to full speed
  right_rate = abs((right_distance > left_distance) ? 1 : (right_distance / left_distance));
  left_rate = abs((right_distance > left_distance) ? (left_distance / right_distance) : 1);

  // move the wheels
  takeUpSlack((right_distance > 0), (left_distance < 0));
  rightMotor.turn(abs(right_distance) * steps_per_mm * settings.moveCalibration, (right_distance > 0), right_rate);
  leftMotor.turn(abs(left_distance) * steps_per_mm * settings.moveCalibration, (left_distance < 0), left_rate);
  wait();
}

boolean Mirobot::ready(){
  return (rightMotor.ready() && leftMotor.ready() && !servo_pulses_left && beepComplete < millis());
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
  if(rightMotor.ready() && leftMotor.ready()){
    int diff = leftLineSensor - rightLineSensor;
    if(diff > 5){
      if(hwVersion == 1){
        right(1);
      }else if(hwVersion == 2){
        left(1);
      }
    }else if(diff < -5){
      if(hwVersion == 1){
        left(1);
      }else if(hwVersion == 2){
        right(1);
      }
    }else{
      forward(5);
    }
  }
}

void Mirobot::collideHandler(){
  readSensors();
  if(_collideStatus == NORMAL){
    if(leftCollide){
      _collideStatus = LEFT_REVERSE;
      back(50);
    }else if(rightCollide){
      _collideStatus = RIGHT_REVERSE;
      back(50);
    }else{
      if(rightMotor.ready() && leftMotor.ready()){
        forward(1000);
      }
    }
  }else if(rightMotor.ready() && leftMotor.ready()){
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
}

void Mirobot::ledHandler(){
  long t = millis();
  uint32_t newLEDColour;
#ifdef AVR
  digitalWrite(STATUS_LED_PIN, (!((t / 100) % 10) || !(((t / 100) - 2) % 10)));
#endif //AVR
#ifdef ESP8266
  if(next_led_pulse < t){
    next_led_pulse = t + 50;

    // On for 300ms every 2000 ms
    if(!((t % 2000) / 300)){
      newLEDColour = marcel.wifi.online ? 0x002200 : 0x000022;
    }else{
      newLEDColour = 0x000000;
    }
    if(newLEDColour != lastLEDColour){
      lastLEDColour = newLEDColour;
      setRGB(newLEDColour, STATUS_LED_PIN);
    }
  }
#endif //ESP8266
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

#ifdef AVR
void Mirobot::readSensors(){
  leftCollide = !digitalRead(LEFT_COLLIDE_SENSOR);
  rightCollide = !digitalRead(RIGHT_COLLIDE_SENSOR);
  leftLineSensor = analogRead(LEFT_LINE_SENSOR);
  rightLineSensor = analogRead(RIGHT_LINE_SENSOR);
}
#endif //AVR

#ifdef ESP8266
void Mirobot::readSensors(){
  uint8_t temp[4];
  if(millis() >= nextADCRead){
    nextADCRead = millis() + 50;
    digitalWrite(LINE_LED_ENABLE, LOW);

    // Fetch the data from the ADC
    Wire.beginTransmission(PCF8591_ADDRESS); // wake up PCF8591
    Wire.write(0x44); // control byte - read ADC0 and increment counter
    Wire.endTransmission();
    Wire.requestFrom(PCF8591_ADDRESS, 5);
    Wire.read(); // Padding bytes to allow conversion to complete
    temp[0] = Wire.read();
    temp[1] = Wire.read();
    temp[2] = Wire.read();
    temp[3] = Wire.read();

    // Sanity check to make sure I2C data hasn't gone out of sync
    if((temp[2] == 0 || temp[2] == 255) && (temp[3] == 0 || temp[3] == 255)){
      leftLineSensor = temp[0];
      rightLineSensor = temp[1];

      leftCollide = !!temp[2];
      rightCollide = !!temp[3];
    }
    digitalWrite(LINE_LED_ENABLE, HIGH);
  }
}
#endif //ESP8266

void Mirobot::sensorNotifier(){
  StaticJsonBuffer<60> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  if(collideNotify){
    collideState_t currentCollideState = collideState();
    if(currentCollideState != lastCollideState){
      lastCollideState = currentCollideState;
      switch(currentCollideState){
        case BOTH:
          outMsg["msg"] = "both";
          marcel.notify("collide", outMsg);
          break;
        case LEFT:
          outMsg["msg"] = "left";
          marcel.notify("collide", outMsg);
          break;
        case RIGHT:
          outMsg["msg"] = "right";
          marcel.notify("collide", outMsg);
          break;
      }
    }
  }
  if(followNotify){
    readSensors();
    int currentFollowState = leftLineSensor - rightLineSensor;
    if(currentFollowState != lastFollowState){
      outMsg["msg"] = currentFollowState;
      marcel.notify("follow", outMsg);
    }
    lastFollowState = currentFollowState;
  }
}

// This allows for runtime configuration of which hardware is used
void Mirobot::version(char v){
  hwVersion = v;
  sprintf(versionStr, "%d.%s", hwVersion, MIROBOT_SUB_VERSION);
  if(v == 1){
    steps_per_mm = STEPS_PER_MM_V1;
    steps_per_degree = STEPS_PER_DEGREE_V1;
    penup_delay = PENUP_DELAY_V1;
    pendown_delay = PENDOWN_DELAY_V1;
    wheel_distance = WHEEL_DISTANCE_V1;
  }else{
    steps_per_mm = STEPS_PER_MM_V2;
    steps_per_degree = STEPS_PER_DEGREE_V2;
    penup_delay = PENUP_DELAY_V2;
    pendown_delay = PENDOWN_DELAY_V2;
    wheel_distance = WHEEL_DISTANCE_V2;
  }
}

void Mirobot::calibrateSlack(unsigned int amount){
  settings.slackCalibration = amount;
  saveSettings();
  calibratingSlack = true;
  rightMotor.turn(1, FORWARD);
  leftMotor.turn(1, BACKWARD);
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
  if(calibratingSlack && rightMotor.ready() && leftMotor.ready()){
    takeUpSlack((rightMotor.lastDirection == FORWARD ? BACKWARD : FORWARD), (leftMotor.lastDirection == FORWARD ? BACKWARD : FORWARD));
  }
}

void Mirobot::checkReady(){
  if(ready()){
    marcel.cmdComplete();
  }
}

#ifdef ESP8266
void Mirobot::updateFirmware(){
  if(marcel.wifi.online){
    ESPhttpUpdate.rebootOnUpdate(true);
    if(ESPhttpUpdate.update("http://downloads.mime.co.uk/Mirobot/v3/mirobot-latest.bin", "") != HTTP_UPDATE_OK){
      Serial.println(ESPhttpUpdate.getLastErrorString());
    }
  }
}

void Mirobot::updateUI(){
  if(marcel.wifi.online){
    ESPhttpUpdate.rebootOnUpdate(false);
    if(ESPhttpUpdate.updateSpiffs("http://downloads.mime.co.uk/Mirobot/v3/ui-latest.bin", "") != HTTP_UPDATE_OK){
      Serial.println(ESPhttpUpdate.getLastErrorString());
    }
  }
}

void Mirobot::updateHandler(){
  if(_updateFWflag){
    _updateFWflag = false;
    updateFirmware();
  }
  if(_updateUIflag){
    _updateUIflag = false;
    updateUI();
  }
}
#endif //ESP8266

void Mirobot::loop(){
  marcel.loop();
  ledHandler();
  servoHandler();
  autoHandler();
  calibrateHandler();
  sensorNotifier();
#ifdef ESP8266
  if(wifiEnabled){
    sendDiscovery();
    updateHandler();
  }
#endif
  checkReady();
}

/*******************************************************
/*
/* Function callback handlers
/*
*******************************************************/

static void _version(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = Mirobot::mainInstance->versionStr;
}

static void _pause(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->pause();
}

static void _resume(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->resume();
}

static void _stop(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->stop();
}

static void _collideState(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  switch(Mirobot::mainInstance->collideState()){
    case NONE:
      outJson["msg"] = "none";
      break;
    case LEFT:
      outJson["msg"] = "left";
      break;
    case RIGHT:
      outJson["msg"] = "right";
      break;
    case BOTH:
      outJson["msg"] = "both";
      break;
  }
}

static void _collideNotify(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->collideNotify = !!strcmp(inJson["arg"], "false");
}

static void _followState(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = Mirobot::mainInstance->followState();
}

static void _followNotify(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->followNotify = !!strcmp(inJson["arg"].asString(), "false");
}

static void _slackCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = Mirobot::mainInstance->settings.slackCalibration;
}

static void _moveCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = Mirobot::mainInstance->settings.moveCalibration;
}

static void _turnCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = Mirobot::mainInstance->settings.turnCalibration;
}

static void _calibrateMove(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->calibrateMove(atof(inJson["arg"].asString()));
}

static void _calibrateTurn(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->calibrateTurn(atof(inJson["arg"].asString()));
}

static void _forward(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->forward(atoi(inJson["arg"].asString()));
}

static void _back(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->back(atoi(inJson["arg"].asString()));
}

static void _right(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->right(atoi(inJson["arg"].asString()));
}

static void _left(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->left(atoi(inJson["arg"].asString()));
}

static void _penup(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->penup();
}

static void _pendown(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->pendown();
}

static void _follow(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->follow();
}

static void _collide(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->collide();
}

static void _beep(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->beep(atoi(inJson["arg"].asString()));
}

static void _calibrateSlack(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->calibrateSlack(atoi(inJson["arg"].asString()));
}

static void _arc(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  if(inJson["arg"].is<JsonArray&>() && inJson["arg"].size() == 2){
    Mirobot::mainInstance->arc(inJson["arg"][0].as<float>(), inJson["arg"][1].as<float>());
  }
}

#ifdef ESP8266
static void _updateFirmware(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->_updateFWflag = true;
}
static void _updateUI(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  Mirobot::mainInstance->_updateUIflag = true;
}
#endif //ESP8266