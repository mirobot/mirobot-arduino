#ifdef ESP8266

#include "Arduino.h"
#include "Mirobot.h"

ShiftStepper motor1(0);
ShiftStepper motor2(1);

MirobotWifi wifi;

CmdManager manager;

WS2812B led(LED_PIN);

static char tmpBuff[10];

Mirobot::Mirobot(){
  blocking = true;
  lastLedChange = millis();
  nextADCRead = 0;
  calibratingSlack = false;
  beepComplete = 0;
  version(3);
}

void Mirobot::begin(){
  ShiftStepper::setup(SHIFT_REG_DATA, SHIFT_REG_CLOCK, SHIFT_REG_LATCH);
  // Set up the pen arm servo
  pinMode(SERVO_PIN, OUTPUT);
  // Set up the collision sensor state
  _collideStatus = NORMAL;
  // Start the pen arm in the up state
  setPenState(UP);
  // Set up the line follower LED enable pin
  pinMode(LINE_LED_ENABLE, OUTPUT);
  // Initialise the settings
  initSettings();
  // Set up the commands for the command manager
  initCmds();
  // Set up the I2C lines for the ADC
  Wire.begin(I2C_DATA, I2C_CLOCK);
}

void Mirobot::enableSerial(){
  blocking = false;
  // Set up Serial and add it to be processed
  Serial.begin(230400);
  Serial.setTimeout(1); 
  manager.addStream(Serial);
}

void Mirobot::enableWifi(){
  wifi.begin(&settings);
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
  manager.addCmd("getConfig",        &Mirobot::_getConfig,        true);
  manager.addCmd("setConfig",        &Mirobot::_setConfig,        true);
  manager.addCmd("resetConfig",      &Mirobot::_resetConfig,      true);
  manager.addCmd("freeHeap",         &Mirobot::_freeHeap,         true);
  manager.addCmd("startWifiScan",    &Mirobot::_startWifiScan,    true);
}

void Mirobot::initSettings(){
  EEPROM.begin(512);
  if(EEPROM.read(EEPROM_OFFSET) == MAGIC_BYTE_1 && EEPROM.read(EEPROM_OFFSET + 1) == MAGIC_BYTE_2 && EEPROM.read(EEPROM_OFFSET + 2) == SETTINGS_VERSION){
    // We've previously written something valid to the EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++){
      *((char*)&settings + t) = EEPROM.read(EEPROM_OFFSET + 2 + t);
    }
  }else{
    settings.settingsVersion = SETTINGS_VERSION;
    settings.slackCalibration = 14;
    settings.moveCalibration = 1.0f;
    settings.turnCalibration = 1.0f;
    settings.sta_ssid[0] = 0;
    settings.sta_pass[0] = 0;
    settings.sta_dhcp = true;
    settings.sta_fixedip = 0;
    settings.sta_fixedgateway = 0;
    settings.sta_fixednetmask = (uint32_t)IPAddress(255, 255, 255, 0);
    settings.sta_fixeddns1 = 0;
    settings.sta_fixeddns2 = 0;
    MirobotWifi::defautAPName(settings.ap_ssid);
    settings.ap_pass[0] = 0;
    settings.discovery = true;
    saveSettings();
  }
}


void Mirobot::_version(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = MIROBOT_VERSION;
}

void Mirobot::_ping(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){}

void Mirobot::_uptime(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = millis();
}

void Mirobot::_pause(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  pause();
}

void Mirobot::_resume(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  resume();
}

void Mirobot::_stop(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  stop();
}

void Mirobot::_collideState(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  //collideState(msg);
}

void Mirobot::_collideNotify(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  collideNotify = !!strcmp(inJson["arg"], "false");
}

void Mirobot::_followState(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  //sprintf(&msg, "%d", followState());
}

void Mirobot::_followNotify(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  followNotify = !!strcmp(inJson["arg"].asString(), "false");
  // Turn the LEDs on or off
  digitalWrite(LINE_LED_ENABLE, followNotify);
}

void Mirobot::_slackCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = settings.slackCalibration;
}

void Mirobot::_moveCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = settings.moveCalibration;
}

void Mirobot::_turnCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = settings.turnCalibration;
}

void Mirobot::_calibrateMove(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  calibrateMove(atof(inJson["arg"].asString()));
}

void Mirobot::_calibrateTurn(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  calibrateTurn(atof(inJson["arg"].asString()));
}

void Mirobot::_forward(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  forward(atoi(inJson["arg"].asString()));
}

void Mirobot::_back(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  back(atoi(inJson["arg"].asString()));
}

void Mirobot::_right(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  right(atoi(inJson["arg"].asString()));
}

void Mirobot::_left(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  left(atoi(inJson["arg"].asString()));
}

void Mirobot::_penup(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  penup();
}

void Mirobot::_pendown(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  pendown();
}

void Mirobot::_follow(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  follow();
}

void Mirobot::_collide(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  collide();
}

void Mirobot::_beep(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  beep(atoi(inJson["arg"].asString()));
}

void Mirobot::_calibrateSlack(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  calibrateSlack(atoi(inJson["arg"].asString()));
}

void Mirobot::_getConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  JsonObject& msg = outJson.createNestedObject("msg");
  const char *modes[] = {"OFF","STA","AP","APSTA"};
  msg["sta_ssid"] = settings.sta_ssid;
  msg["sta_dhcp"] = settings.sta_dhcp;
  msg["sta_rssi"] = MirobotWifi::getStaRSSI();
  if(!settings.sta_dhcp){
    msg["sta_fixedip"] = IPAddress(settings.sta_fixedip).toString();
    msg["sta_fixedgateway"] = IPAddress(settings.sta_fixedgateway).toString();
    msg["sta_fixednetmask"] = IPAddress(settings.sta_fixednetmask).toString();
  }
  msg["sta_ip"] = MirobotWifi::getStaIp().toString();
  msg["ap_ssid"] = settings.ap_ssid;
  msg["ap_encrypted"] = !!strlen(settings.ap_pass);
  msg["discovery"] = settings.discovery;
  msg["wifi_mode"] = modes[MirobotWifi::getWifiMode()];
}

void Mirobot::_setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  IPAddress addr;
  if(!(inJson.containsKey("arg") && inJson["arg"].is<JsonObject&>())) return;

  // Set the SSID to connect to
  if(inJson["arg"].asObject().containsKey("sta_ssid")){
    strcpy(settings.sta_ssid, inJson["arg"]["sta_ssid"]);
  }
  // Set the password for the SSID
  if(inJson["arg"].asObject().containsKey("sta_pass")){
    strcpy(settings.sta_pass, inJson["arg"]["sta_pass"]);
  }
  // Change the name of the built in access point
  if(inJson["arg"].asObject().containsKey("ap_ssid")){
    strcpy(settings.ap_ssid, inJson["arg"]["ap_ssid"]);
  }
  // Set the password for the access point
  if(inJson["arg"].asObject().containsKey("ap_pass")){
    strcpy(settings.ap_pass, inJson["arg"]["ap_pass"]);
  }
  // Set whether to use DHCP
  if(inJson["arg"].asObject().containsKey("sta_dhcp")){
    settings.sta_dhcp = inJson["arg"]["sta_dhcp"];
  }
  // Use a fixed IP address
  if(inJson["arg"].asObject().containsKey("sta_fixedip")){
    addr.fromString(inJson["arg"]["sta_fixedip"].asString());
    settings.sta_fixedip = addr;
  }
  // The DNS server to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixedgateway")){
    addr.fromString(inJson["arg"]["sta_fixedgateway"].asString());
    settings.sta_fixedgateway = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixednetmask")){
    addr.fromString(inJson["arg"]["sta_fixednetmask"].asString());
    settings.sta_fixednetmask = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixeddns1")){
    addr.fromString(inJson["arg"]["sta_fixeddns1"].asString());
    settings.sta_fixeddns1 = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixeddns2")){
    addr.fromString(inJson["arg"]["sta_fixeddns2"].asString());
    settings.sta_fixeddns2 = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("discovery")){
    settings.discovery = inJson["arg"]["discovery"];
  }
  wifi.setupWifi();
  saveSettings();
}
void Mirobot::_resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  settings.settingsVersion = 0;
  saveSettings();
  initSettings();
  wifi.setupWifi();
}
void Mirobot::_freeHeap(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = ESP.getFreeHeap();
}
void Mirobot::_startWifiScan(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  MirobotWifi::startWifiScan();
}

void Mirobot::saveSettings(){
  EEPROM.write(EEPROM_OFFSET, MAGIC_BYTE_1);
  EEPROM.write(EEPROM_OFFSET + 1, MAGIC_BYTE_2);
  for (unsigned int t=0; t<sizeof(settings); t++){
    EEPROM.write(EEPROM_OFFSET + 2 + t, *((char*)&settings + t));
  }
  EEPROM.commit();
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
  readADC();
  return leftLineSensor - rightLineSensor;
}

void Mirobot::collide(){
  colliding = true;
}

void Mirobot::collideState(char &state){
  readADC();

  if(leftCollide && rightCollide){
    strcpy(&state, "both");
  }else if(leftCollide){
    strcpy(&state, "left");
  }else if(rightCollide){
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
  readADC();

  if(motor1.ready() && motor2.ready()){
    int diff = leftLineSensor - rightLineSensor;
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
  readADC();

  if(_collideStatus == NORMAL){
    if(leftCollide){
      _collideStatus = LEFT_REVERSE;
      back(50);
    }else if(rightCollide){
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
}

void Mirobot::ledHandler(){
  uint8_t val = (abs((millis() % (uint32_t)LED_PULSE_TIME) - LED_PULSE_TIME/2) / (LED_PULSE_TIME/2)) * 50;
  led.setRGBA(LED_COLOUR_NORMAL, val);
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

void Mirobot::readADC(){
  uint8_t temp[4];
  if(millis() >= nextADCRead){
    nextADCRead = millis() + 10;

    // Fetch the data from the ADC
    Wire.beginTransmission(PCF8591_ADDRESS); // wake up PCF8591
    Wire.write(0x04); // control byte - read ADC0 and increment counter
    Wire.endTransmission();
    Wire.requestFrom(PCF8591_ADDRESS, 6);
    Wire.read(); // Padding bytes to allow conversion to complete
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
  }
}

void Mirobot::sensorNotifier(){
  StaticJsonBuffer<60> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  if(collideNotify){
    readADC();
    char currentCollideState = rightCollide | (leftCollide << 1);
    if(currentCollideState != lastCollideState){
      if(leftCollide && rightCollide){
        outMsg["msg"] = "both";
        manager.notify("collide", outMsg);
      }else if(leftCollide){
        outMsg["msg"] = "left";
        manager.notify("collide", outMsg);
      }else if(rightCollide){
        outMsg["msg"] = "right";
        manager.notify("collide", outMsg);
      }
      lastCollideState = currentCollideState;
    }
  }
  if(followNotify){
    readADC();
    int currentFollowState = leftLineSensor - rightLineSensor;
    if(currentFollowState != lastFollowState){
      outMsg["msg"] = currentFollowState;
      manager.notify("follow", outMsg);
    }
    lastFollowState = currentFollowState;
  }
}

void Mirobot::networkNotifier(){
  if(!MirobotWifi::networkChanged) return;
  StaticJsonBuffer<500> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  MirobotWifi::networkChanged = false;
  _getConfig(outMsg, outMsg);
  manager.notify("network", outMsg);
}

void Mirobot::wifiScanNotifier(){
  if(!MirobotWifi::wifiScanReady) return;
  StaticJsonBuffer<1000> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  JsonArray& msg = outMsg.createNestedArray("msg");
  MirobotWifi::wifiScanReady = false;
  wifi.getWifiScanData(msg);
  manager.notify("wifiScan", outMsg);
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
  networkNotifier();
  wifiScanNotifier();
  checkReady();
  manager.process();
  wifi.run();
}

#endif