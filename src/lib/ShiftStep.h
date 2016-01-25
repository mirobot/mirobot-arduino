// Version 72503c42 from github.com/bjpirt/HotStepper

#ifndef ShiftStep_h
#define ShiftStep_h
#include "Arduino.h"


#define FORWARD 1
#define BACKWARD 0

#define TIMER1INT 1
#define TIMER2INT 2

class ShiftStep {
  public:
    ShiftStep(int, int, int);
    static void setup();
    void turn(long steps, byte direction);
    boolean ready();
    long remaining();
    void release();
    void pause();
    void resume();
    void stop();
    byte lastDirection;
  private:
    boolean _paused;
    volatile long _remaining;
    byte _dir;
    byte nextStep();
    void setStep(byte);
    void setNextStep();
};

#endif
