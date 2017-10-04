#include "pcf8591.h"

PCF8591::PCF8591(int data, int clock, unsigned char addr){
  Wire.begin(data, clock);
  address = addr;
}

void PCF8591::readSensors(unsigned char * values){
  // Fetch the data from the ADC
  Wire.beginTransmission(address); // wake up PCF8591
  Wire.write(0x44); // control byte - read ADC0 and increment counter
  Wire.endTransmission();
  Wire.requestFrom(address, 5);
  Wire.read(); // Padding bytes to allow conversion to complete
  values[0] = Wire.read();
  values[1] = Wire.read();
  values[2] = Wire.read();
  values[3] = Wire.read();
}