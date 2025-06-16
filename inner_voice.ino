#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <SoftwareSerial.h>

// --- Bluetooth via SoftwareSerial ---
SoftwareSerial btSerial(10, 11); // RX, TX

// --- ADXL345 Configuration ---
#define ADXL345 0x53
float threshold = 0.2;
unsigned long lastStepTime = 0;
int stepCount = 0;
float prevAccelMag = 0;
float prevFiltered = 0;
const float alpha = 0.8;

// --- MAX30100 Configuration ---
PulseOximeter pox;
uint32_t lastHRReport = 0;

// --- Setup ---
void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  Wire.begin();

  // --- Check ADXL345 Connection ---
  Wire.beginTransmission(ADXL345);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(ADXL345, 1);
  if (Wire.available()) {
    byte devid = Wire.read();
    if (devid == 0xE5) {
      Serial.println("ADXL345 connected and responding.");
    } else {
      Serial.print("Unexpected ADXL345 DEVID: 0x");
      Serial.println(devid, HEX);
      while (1);
    }
  } else {
    Serial.println("ADXL345 not responding. Check wiring!");
    while (1);
  }

  // --- Init ADXL345 ---
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D); // Power control
  Wire.write(0x08); // Measurement mode
  Wire.endTransmission();

  Wire.beginTransmission(ADXL345);
  Wire.write(0x31); // Data format
  Wire.write(0x00); // ±2g range
  Wire.endTransmission();

  Serial.println("ADXL345 initialized");

  // --- Init MAX30100 ---
  if (!pox.begin()) {
    Serial.println("MAX30100 not found. Check wiring!");
    while (1);
  }
  Serial.println("MAX30100 initialized");
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

// --- Loop ---
void loop() {
  readADXL345();
  detectStep();
  pox.update();  // MAX30100 update

  if (millis() - lastHRReport > 1000) {
    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    Serial.print("HR: "); Serial.print(hr); Serial.print(" bpm | ");
    Serial.print("SpO2: "); Serial.print(spo2); Serial.print("% | ");
    Serial.print("Steps: "); Serial.println(stepCount);

    // Send to Bluetooth
    btSerial.print("HR: "); btSerial.print(hr); btSerial.print(" bpm, ");
    btSerial.print("SpO2: "); btSerial.print(spo2); btSerial.print("%, ");
    btSerial.print("Steps: "); btSerial.println(stepCount);

    lastHRReport = millis();
  }

  delay(20); // ~50Hz sampling
}

// --- Read Accelerometer ---
int16_t x, y, z;
void readADXL345() {
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);
  if (Wire.available() == 6) {
    x = Wire.read() | (Wire.read() << 8);
    y = Wire.read() | (Wire.read() << 8);
    z = Wire.read() | (Wire.read() << 8);
  }
}

// --- Step Detection Logic ---
void detectStep() {
  float ax = x * 0.0039;
  float ay = y * 0.0039;
  float az = z * 0.0039;
  float accMag = sqrt(ax * ax + ay * ay + az * az);

  float filtered = alpha * prevFiltered + (1 - alpha) * (accMag - prevAccelMag);
  prevAccelMag = accMag;
  prevFiltered = filtered;

  if (filtered > threshold && (millis() - lastStepTime) > 300) {
    stepCount++;
    lastStepTime = millis();
  }
}

// --- Beat Callback ---
void onBeatDetected() {
  Serial.println("Beat detected");
}
