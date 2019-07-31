// Conmutación del multiplexor TCA9548A I2C
// 

#include "TCA9548A.h"


void tcaselect(int dir, int canal) {
  if (canal > 7) return;
  Wire.beginTransmission(dir);
  Wire.write(1 << canal);
  Wire.endTransmission();
}