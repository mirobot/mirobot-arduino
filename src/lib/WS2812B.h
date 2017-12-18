#ifndef __WS2812B_H__
#define __WS2812B_H__

#ifdef ESP8266
static void send0(uint8_t p) {
  uint8_t i;
  i = 4; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << p);
  i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << p);
}

static void send1(uint8_t p) {
  uint8_t i;
  i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << p);
  i = 6; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << p);
}

static void setRGB(uint32_t rgb, int pin){
  uint32_t grb = (rgb << 8 & 0xFF0000) | (rgb >> 8 & 0x00FF00) | (rgb & 0xFF);
  uint32_t mask = 0x800000;
  os_intr_lock();
  while (mask) {
    (grb & mask) ? send1(pin) : send0(pin);
    mask >>= 1;
  }
  os_intr_unlock();
}

static void setRGBA(uint32_t rgb, uint8_t alpha, int pin){
  float factor = alpha / 255.0;
  uint8_t r = ((rgb >> 16) & 0xFF) * factor;
  uint8_t g = ((rgb >> 8) & 0xFF) * factor;
  uint8_t b = (rgb & 0xFF) * factor;
  setRGB((r << 16) | (g << 8) | b, pin);
}
#endif //ESP8266

#endif