#ifndef __ShiftStepper_h__
#define __ShiftStepper_h__
#include "Arduino.h"

#define FORWARD 1
#define BACKWARD 0

#define BASE_INTERRUPT_US 50
#define DEFAULT_STEP_PERIOD 3000
#define UCOUNTER_DEFAULT DEFAULT_STEP_PERIOD/BASE_INTERRUPT_US

class ShiftStepper {
  public:
    ShiftStepper(int);
    static void setup(int, int, int);
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
    byte lastDirection;
  private:
    static ShiftStepper *firstInstance;
    ShiftStepper *nextInstance;
    void addNext(ShiftStepper *ref);
    boolean _paused;
    byte _pinmask;
    volatile long _remaining;
    byte _dir;
    byte nextStep();
    void setStep(byte);
    void setNextStep();
    void trigger();
    byte currentStep;
    static int data_pin;
    static int clock_pin;
    static int latch_pin;
    int microCounter;
    int motor_offset;
    static uint8_t lastBits;
    static uint8_t currentBits;
    void updateBits(uint8_t bits);
    static void sendBits();
    static void startTimer();
    static void stopTimer();
};

#endif
