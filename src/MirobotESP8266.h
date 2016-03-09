#ifdef ESP8266
#ifndef __MirobotESP8266_h__
#define __MirobotESP8266_h__

#include "Arduino.h"
#include "lib/ShiftStepper.h"
#include "lib/CmdManager.h"
#include "lib/MirobotWifi.h"
#include "lib/WS2812B.h"
#include "Wire.h"
#include <EEPROM.h>
#include "./lib/ArduinoJson/ArduinoJson.h"

class CmdManager;
struct CmdResult;

#define STEPS_PER_TURN    2048.0f

#define CIRCUMFERENCE_MM_V1  251.3f
#define WHEEL_DISTANCE_V1    126.0f
#define PENUP_DELAY_V1 1200
#define PENDOWN_DELAY_V1 2000

#define CIRCUMFERENCE_MM_V2  256.0f
#define WHEEL_DISTANCE_V2    120.0f
#define PENUP_DELAY_V2 2000
#define PENDOWN_DELAY_V2 1000

#define STEPS_PER_MM_V1      STEPS_PER_TURN / CIRCUMFERENCE_MM_V1
#define STEPS_PER_DEGREE_V1  ((WHEEL_DISTANCE_V1 * 3.1416) / 360) * STEPS_PER_MM_V1
#define STEPS_PER_MM_V2      STEPS_PER_TURN / CIRCUMFERENCE_MM_V2
#define STEPS_PER_DEGREE_V2  ((WHEEL_DISTANCE_V2 * 3.1416) / 360) * STEPS_PER_MM_V2


#define SERVO_PIN 4
#define SERVO_PULSES 15

#define NOTE_C4  262

#define EEPROM_OFFSET 0
#define MAGIC_BYTE_1 0xF0
#define MAGIC_BYTE_2 0x0D
#define SETTINGS_VERSION 1

#define SHIFT_REG_DATA  12
#define SHIFT_REG_CLOCK 13
#define SHIFT_REG_LATCH 14

#define MIROBOT_VERSION "3.0.0"

#define SPEAKER_PIN 5

#define LINE_LED_ENABLE 16

#define LED_PIN 15
#define LED_PULSE_TIME 6000.0
#define LED_COLOUR_NORMAL 0xFFFFFF

#define PCF8591_ADDRESS B1001000
#define I2C_DATA  0
#define I2C_CLOCK 2


typedef enum {UP, DOWN} penState_t;

typedef enum {NORMAL, RIGHT_REVERSE, RIGHT_TURN, LEFT_REVERSE, LEFT_TURN} collideStatus_t;

struct MirobotSettings {
  uint8_t      settingsVersion;
  unsigned int slackCalibration;
  float        moveCalibration;
  float        turnCalibration;
  char         sta_ssid[32];
  char         sta_pass[64];
  bool         sta_dhcp;
  uint32_t     sta_fixedip;
  uint32_t     sta_fixedgateway;
  uint32_t     sta_fixednetmask;
  uint32_t     sta_fixeddns1;
  uint32_t     sta_fixeddns2;
  char         ap_ssid[32];
  char         ap_pass[64];
  char         ap_auth_mode;
  char         ap_channel;
  bool         discovery;
};

class Mirobot {
  public:
    Mirobot();
    void begin();
    void enableSerial();
    void enableWifi();
    void forward(int distance);
    void back(int distance);
    void right(int angle);
    void left(int angle);
    void penup();
    void pendown();
    void pause();
    void resume();
    void stop();
    void reset();
    void follow();
    int  followState();
    void collide();
    void collideState(char &state);
    void beep(int);
    void setHwVersion(char&);
    boolean ready();
    void process();
    void version(char);
    void calibrateSlack(unsigned int);
    void calibrateMove(float);
    void calibrateTurn(float);
    char versionNum;
    MirobotSettings settings;
    boolean blocking;
    boolean collideNotify;
    boolean followNotify;
  private:
    void wait();
    void followHandler();
    void collideHandler();
    void ledHandler();
    void servoHandler();
    void autoHandler();
    void sensorNotifier();
    void checkState();
    void initCmds();
    void initSettings();
    void saveSettings();
    char lastCollideState;
    int lastFollowState;
    collideStatus_t _collideStatus;
    unsigned long lastLedChange;
    Mirobot& self() { return *this; }
    penState_t penState;
    void setPenState(penState_t);
    void takeUpSlack(byte, byte);
    void calibrateHandler();
    unsigned long next_servo_pulse;
    unsigned char servo_pulses_left;
    boolean paused;
    boolean following;
    boolean colliding;
    float steps_per_mm;
    float steps_per_degree;
    int penup_delay;
    int pendown_delay;
    long beepComplete;
    boolean calibratingSlack;
    void checkReady();
    void _version(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _ping(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _uptime(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _pause(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _resume(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _stop(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _collideState(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _collideNotify(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _followState(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _followNotify(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _slackCalibration(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _moveCalibration(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _turnCalibration(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _calibrateMove(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _calibrateTurn(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _forward(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _back(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _right(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _left(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _penup(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _pendown(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _follow(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _collide(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _beep(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _calibrateSlack(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _getConfig(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _setConfig(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _resetConfig(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void _freeHeap(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
    void readADC();
    boolean leftCollide;
    boolean rightCollide;
    uint8_t leftLineSensor;
    uint8_t rightLineSensor;
    long nextADCRead;
};

#endif
#endif
