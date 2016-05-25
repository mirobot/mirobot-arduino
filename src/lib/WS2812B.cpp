#ifdef ESP8266
#include "WS2812B.h"

WS2812B::WS2812B(int pinNo){
  pin = pinNo;
  pinMode(pin, OUTPUT);
}

void WS2812B::setRGB(uint32_t rgb){
  uint32_t grb = (rgb << 8 & 0xFF0000) | (rgb >> 8 & 0x00FF00) | (rgb & 0xFF);
  uint32_t mask = 0x800000;
  os_intr_lock();
  while (mask) {
    (grb & mask) ? send1(pin) : send0(pin);
    mask >>= 1;
  }
  os_intr_unlock();
}

void WS2812B::setRGBA(uint32_t rgb, uint8_t alpha){
  float factor = alpha / 255.0;
  uint8_t r = ((rgb >> 16) & 0xFF) * factor;
  uint8_t g = ((rgb >> 8) & 0xFF) * factor;
  uint8_t b = (rgb & 0xFF) * factor;
  setRGB((r << 16) | (g << 8) | b);
}

void WS2812B::send0(uint8_t p) {
  uint8_t i;
  i = 4; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << p);
  i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << p);
}

void WS2812B::send1(uint8_t p) {
  uint8_t i;
  i = 8; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << p);
  i = 6; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << p);
}
#endif