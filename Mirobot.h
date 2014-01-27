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

class Mirobot {
  public:
    Mirobot();
    void setup();
    void forward(int distance);
    void back(int distance);
    void right(int angle);
    void left(int angle);
    void penup();
    void pendown();
    boolean ready();
    void setBlocking(boolean val);
    void useWebSockets(Stream &s);
    void useRawSockets(Stream &s);
    void processInput();
  private:
    void wait();
    void setupCmdProcessor(char type, Stream &s);
    boolean blocking;
    Mirobot& self() { return *this; }
};


#endif
