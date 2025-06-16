#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("♥ Beat!");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing MAX30100...");

  if (!pox.begin()) {
    Serial.println("MAX30100 not found. Check wiring/power.");
    while (1);
  }

  // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6); // Set LED current (adjust for performance)
  pox.setOnBeatDetectedCallback(onBeatDetected);

  Serial.println("MAX30100 ready.");
}

void loop() {
  pox.update();  // Required to read sensor continuously

  // Print data every second
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart Rate (BPM): ");
    Serial.print(pox.getHeartRate());
    Serial.print(" | SpO2 (%): ");
    Serial.println(pox.getSpO2());

    tsLastReport = millis();
  }
}
