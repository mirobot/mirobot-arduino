#include "Wire.h"

class PCF8591{
  public:
    PCF8591(int, int, unsigned char);
    void begin();
    void readSensors(unsigned char * values);
  private:
    unsigned char address;
};
