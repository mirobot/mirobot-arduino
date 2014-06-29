#ifndef Mirobot_h
#define Mirobot_h

#include "Arduino.h"
#include "HotStepper.h"
#include "PWMServo.h"
#include "CmdProcessor.h"

#define STEPS_PER_TURN    2048.0
#define CIRCUMFERENCE_MM  251.3
#define STEPS_PER_MM      STEPS_PER_TURN / CIRCUMFERENCE_MM
#define WHEEL_DISTANCE    126.0
#define STEPS_PER_DEGREE  ((WHEEL_DISTANCE * 3.1416) / 360) * STEPS_PER_MM

#define WIFI_RESET A4
#define WIFI_READY A5
#define STATUS_LED 8

#define MIROBOT_VERSION "20140629"


typedef enum {POWERED_UP, CONNECTED} mainState_t;

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
    boolean ready();
    void setBlocking(boolean val);
    void processInput();
  private:
    void wait();
    void ledHandler();
    void checkState();
    mainState_t mainState;
    unsigned long lastLedChange;
    boolean blocking;
    Mirobot& self() { return *this; }
};


#endif
