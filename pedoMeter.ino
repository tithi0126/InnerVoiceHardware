#include <Wire.h>

#define ADXL345 0x53

float threshold = 1.2;  // Acceleration threshold (tweak if needed)
unsigned long lastStepTime = 0;
int stepCount = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Set ADXL345 to measurement mode
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D);  // Power control register
  Wire.write(0x08);  // Measurement mode
  Wire.endTransmission();

  Serial.println("Pedometer Started");
}

void loop() {
  int x, y, z;
  float acc;

  // Read 6 bytes from ADXL345
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);

  x = Wire.read() | (Wire.read() << 8);
  y = Wire.read() | (Wire.read() << 8);
  z = Wire.read() | (Wire.read() << 8);

  // Convert to 'g' units (1g = 9.8 m/s²; sensitivity ~256 LSB/g at default 2g range)
  float xf = x * 0.0039;
  float yf = y * 0.0039;
  float zf = z * 0.0039;

  acc = sqrt(xf * xf + yf * yf + zf * zf);  // total acceleration

  // Step detection based on acceleration peak
  if (acc > threshold && (millis() - lastStepTime > 300)) {
    stepCount++;
    lastStepTime = millis();
    Serial.print("Step Detected! Total Steps: ");
    Serial.println(stepCount);
  }

  delay(50);  // Sampling every 50 ms
}
