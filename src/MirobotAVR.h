#ifndef __MirobotAVR_h__
#define __MirobotAVR_h__

#include "Arduino.h"
#include "lib/HotStepper.h"
#include "lib/CmdProcessor.h"
#include <EEPROM.h>

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

#define SERVO_PULSES 15

#define NOTE_C4  262

#define MAGIC_BYTE_1 0xF0
#define MAGIC_BYTE_2 0x0D

#define STATUS_LED 13

#define MIROBOT_VERSION "2.0.5"

#define EEPROM_OFFSET 16

#define SERVO_PIN 3

#define SPEAKER_PIN 9

#define LEFT_LINE_SENSOR  A0
#define RIGHT_LINE_SENSOR A1

#define LEFT_COLLIDE_SENSOR  A3
#define RIGHT_COLLIDE_SENSOR A2

typedef enum {UP, DOWN} penState_t;

typedef enum {NORMAL, RIGHT_REVERSE, RIGHT_TURN, LEFT_REVERSE, LEFT_TURN} collideState_t;

struct Settings {
  byte         hwmajor;
  byte         hwminor;
  unsigned int slackCalibration;
  float        moveCalibration;
  float        turnCalibration;
};

class Mirobot {
  public:
    Mirobot();
    void setup();
    void setup(Stream &s);
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
    Settings settings;
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
    void initSettings();
    void saveSettings();
    char lastCollideState;
    int lastFollowState;
    collideState_t _collideState;
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
};

#endif
