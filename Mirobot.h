#ifndef Mirobot_h
#define Mirobot_h

#define FROM_LIB
#include "Arduino.h"
#include "HotStepper.h"
#include "CmdProcessor.h"

#define STEPS_PER_TURN    2048.0
#define CIRCUMFERENCE_MM  251.3
#define STEPS_PER_MM      STEPS_PER_TURN / CIRCUMFERENCE_MM
#define WHEEL_DISTANCE    126.0
#define STEPS_PER_DEGREE  ((WHEEL_DISTANCE * 3.1416) / 360) * STEPS_PER_MM

#define WIFI_RESET 3
#define WIFI_READY 2
#define STATUS_LED 13

#define MIROBOT_VERSION "20141020"

#define SERVO_PIN 3
#define SERVO_PULSES 15

#define SPEAKER_PIN 9
#define NOTE_C4  262

#define LEFT_LINE_SENSOR  A0
#define RIGHT_LINE_SENSOR A1

#define LEFT_COLLIDE_SENSOR  A2
#define RIGHT_COLLIDE_SENSOR A3

typedef enum {POWERED_UP, CONNECTED} mainState_t;

typedef enum {UP, DOWN} penState_t;

class Mirobot {
  public:
    Mirobot();
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
    void collide();
    void beep(int);
    boolean ready();
    void setBlocking(boolean val);
    void process();
  private:
    void wait();
    void followHandler();
    void collideHandler();
    void ledHandler();
    void servoHandler();
    void autoHandler();
    void checkState();
    mainState_t mainState;
    unsigned long lastLedChange;
    boolean blocking;
    Mirobot& self() { return *this; }
    penState_t penState;
    void setPenState(penState_t);
    unsigned long next_servo_pulse;
    unsigned char servo_pulses_left;
    boolean paused;
    boolean following;
    boolean colliding;
};

#endif
