// Version 72503c42 from github.com/bjpirt/HotStepper

#ifndef HotStepper_h
#define HotStepper_h
#include "Arduino.h"


#define FORWARD 1
#define BACKWARD 0

#define TIMER1INT 1
#define TIMER2INT 2

#define HOTSTEPPER_TIMER1

class HotStepper {
  public:
    HotStepper(volatile uint8_t* port, byte offset);
    static void setup();
    static void setup(char timer);
    void instanceSetup();
    void turn(long steps, byte direction);
    boolean ready();
    long remaining();
    void release();
    static void triggerTop();
    void pause();
    void resume();
    void stop();
    byte lastDirection;
  private:
    static HotStepper *firstInstance;
    HotStepper *nextInstance;
    void addNext(HotStepper *ref);
    boolean _paused;
    volatile uint8_t* _port;
    byte _pinmask;
    volatile long _remaining;
    byte _dir;
    byte nextStep();
    void setStep(byte);
    void setNextStep();
    void trigger();
    byte pad(byte, byte);
    byte unpad(byte, byte);
};

#endif
