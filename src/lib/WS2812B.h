#ifndef __WS2812B_H__
#define __WS2812B_H__

#include "Arduino.h"

class WS2812B {
  public:
    WS2812B(int);
    void setRGB(uint32_t);
    void setRGBA(uint32_t, uint8_t);
  private:
    int pin;
    void send0(uint8_t);
    void send1(uint8_t);
};

#endif
