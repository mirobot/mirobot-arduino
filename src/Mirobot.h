#ifndef Mirobot_h
#define Mirobot_h

#include "Arduino.h"
#include <Marceau.h>
#include <EEPROM.h>

#ifdef AVR
#include "lib/HotStepper.h"
#endif

#ifdef ESP8266
#include "lib/ShiftStepper.h"
#include "lib/Discovery.h"
#include "lib/WS2812B.h"
#include <Wire.h>
#include <ESP8266httpUpdate.h>
#endif

#define SERIAL_BUFFER_LENGTH 180

// The steppers have a gear ratio of 1:63.7 and have 32 steps per turn. 32 x 63.7 = 2038.4
#define STEPS_PER_TURN    2038.0f

#define CIRCUMFERENCE_MM_V1  251.3f
#define WHEEL_DISTANCE_V1    126.0f
#define PENUP_DELAY_V1 1200
#define PENDOWN_DELAY_V1 2000

#define CIRCUMFERENCE_MM_V2  256.0f
#define WHEEL_DISTANCE_V2    120.0f
#define PENUP_DELAY_V2 2000
#define PENDOWN_DELAY_V2 1100

#define STEPS_PER_MM_V1      STEPS_PER_TURN / CIRCUMFERENCE_MM_V1
#define STEPS_PER_DEGREE_V1  ((WHEEL_DISTANCE_V1 * 3.1416) / 360) * STEPS_PER_MM_V1
#define STEPS_PER_MM_V2      STEPS_PER_TURN / CIRCUMFERENCE_MM_V2
#define STEPS_PER_DEGREE_V2  ((WHEEL_DISTANCE_V2 * 3.1416) / 360) * STEPS_PER_MM_V2


#define MIROBOT_SUB_VERSION "1.1"

#define EEPROM_OFFSET 0
#define MAGIC_BYTE_1 0xF0
#define MAGIC_BYTE_2 0x0D
#define SETTINGS_VERSION 2

#define NOTE_C4  262

#define SERVO_PULSES 15

#ifdef AVR
  #define SERVO_PIN 3

  #define SPEAKER_PIN 9

  #define STATUS_LED_PIN 13

  #define LEFT_LINE_SENSOR  A0
  #define RIGHT_LINE_SENSOR A1

  #define LEFT_COLLIDE_SENSOR  A3
  #define RIGHT_COLLIDE_SENSOR A2
#endif

#ifdef ESP8266
  #define SERVO_PIN 4

  #define SHIFT_REG_DATA  12
  #define SHIFT_REG_CLOCK 13
  #define SHIFT_REG_LATCH 14

  #define SPEAKER_PIN 5

  #define LINE_LED_ENABLE 16

  #define STATUS_LED_PIN 15

  #define PCF8591_ADDRESS B1001000
  #define I2C_DATA  0u
  #define I2C_CLOCK 2u
#endif

typedef enum {UP, DOWN} penState_t;
typedef enum {NONE=0, RIGHT, LEFT, BOTH} collideState_t;
typedef enum {NORMAL, RIGHT_REVERSE, RIGHT_TURN, LEFT_REVERSE, LEFT_TURN} collideStatus_t;

struct MirobotSettings {
  uint8_t      settingsVersion;
  unsigned int slackCalibration;
  float        moveCalibration;
  float        turnCalibration;
#ifdef ESP8266
  bool         discovery;
#endif
};

static void _version(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _pause(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _resume(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _stop(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _collideState(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _collideNotify(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _followState(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _followNotify(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _slackCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _moveCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _turnCalibration(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _calibrateMove(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _calibrateTurn(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _forward(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _back(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _right(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _left(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _penup(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _pendown(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _follow(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _collide(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _beep(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _calibrateSlack(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _arc(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
#ifdef ESP8266
static void _updateFirmware(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
static void _updateUI(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
#endif //ESP8266

class Mirobot {
  public:
    Mirobot();
    void begin();
    void begin(unsigned char);
    void enableSerial();
    void forward(int distance);
    void back(int distance);
    void right(int angle);
    void left(int angle);
    void penup();
    void pendown();
    void pause();
    void resume();
    void stop();
    void follow();
    int  followState();
    void collide();
    collideState_t collideState();
    void beep(int);
    void arc(float, float);
    boolean ready();
    void loop();
    void calibrateSlack(unsigned int);
    void calibrateMove(float);
    void calibrateTurn(float);
    static Mirobot getMeArmInstance();
#ifdef ESP8266
    void enableWifi();
    void updateFirmware();
    void updateHandler();
    void updateUI();
    boolean _updateFWflag = false;
    boolean _updateUIflag = false;
#endif
    static Mirobot * mainInstance;
    char hwVersion;
    char versionStr[9];
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
    void readSensors();
    void sensorNotifier();
    void checkState();
    void initSettings();
    void saveSettings();
    void checkReady();
    void version(char);
    void initCmds();
    void beginSerial();
#ifdef ESP8266
    void generateAPName(char * name);
    void beginWifi();
    void sendDiscovery();
#endif
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
    unsigned long next_led_pulse = 0;
    unsigned char servo_pulses_left;
    boolean paused;
    boolean following;
    boolean colliding;
    float steps_per_mm;
    float steps_per_degree;
    int penup_delay;
    int pendown_delay;
    int wheel_distance;
    long beepComplete;
    boolean calibratingSlack = false;
    long nextADCRead;
    bool serialEnabled = false;
    unsigned long last_char;
    char serial_buffer[SERIAL_BUFFER_LENGTH];
    int serial_buffer_pos;
    boolean leftCollide;
    boolean rightCollide;
    uint8_t leftLineSensor;
    uint8_t rightLineSensor;
    boolean wifiEnabled = false;
    long nextDiscovery = 0;
    uint32_t lastLEDColour;
};

#endif
