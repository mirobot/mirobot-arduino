#ifndef __HotStepper_h__
#define __HotStepper_h__

#include "Arduino.h"

#define FORWARD 1
#define BACKWARD 0

#define PULSE_PER_3MS 100

class HotStepper {
  public:
    HotStepper(volatile uint8_t* port, byte offset);
    static void begin();
    void instanceSetup();
    void turn(long steps, byte direction);
    void turn(long steps, byte direction, float rate);
    boolean ready();
    long remaining();
    void release();
    static void triggerTop();
    void pause();
    void resume();
    void stop();
    boolean checkReady();
    byte lastDirection;
  private:
    static HotStepper *firstInstance;
    HotStepper *nextInstance;
    void addNext(HotStepper *ref);
    boolean _paused;
    volatile uint8_t* _port;
    byte _pinmask;
    volatile long _remaining;
    static void disableInterrupts();
    static void enableInterrupts();
    byte _dir;
    byte nextStep();
    void setStep(byte);
    void setNextStep();
    void trigger();
    byte pad(byte, byte);
    byte unpad(byte, byte);
    int _counterMax;
    int counter;
};

#endif
