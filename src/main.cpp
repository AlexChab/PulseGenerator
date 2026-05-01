// esp32-generator
// This code implements a PWM and PULSE signal generator on an ESP32, allowing users to set frequency and duty cycle via the Serial Monitor.
// using internal pwm on esp32
// Author:  AlexChab with contributions from ChatGPT
// License: MIT License

#include <Arduino.h>

// Pin Settings
const int outputPin = 18;
const int ledPin = 2;  // internal LED (indicator)

// Internal PWM Settings
const int ledcChannel = 0;

// set PWM reesolution and frequency range

const int pwmResolution = 8;  // 8 bits give a range of 0-255

// set pulse resolution setings
const int pulseResolution = 13;  // Resolution of 13 bits (from 0 to 8191)

// Current values for default PWM startup

uint32_t currentFreq = 5000;
int currentDutyPercent = 45;

void updatePWM(uint32_t f, int percent) {
  // Constrain the percentage for safety
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  // Convert percentages (0-100) to register values (0-255)
  int dutyValue = map(percent, 0, 100, 0, 255);

  // Apply the settings
  ledcSetup(ledcChannel, f, pwmResolution);
  ledcWrite(ledcChannel, dutyValue);

  Serial.printf("Updated: %u Hz | Duty Cycle: %d%%\n", f, percent);
}

void setPulse(uint32_t periodMs, uint32_t pulseWidthMs) {
  // Constrain the input data for safety

  Serial.printf("Function Set PULSE: Period = %u ms, Pulse Width = %u ms\n", periodMs, pulseWidthMs);
  periodMs = constrain(periodMs, 10, 3000);
  pulseWidthMs = constrain(pulseWidthMs, 0, periodMs);

  // Calculate the frequency (Hz)
  double frequency = 1000.0 / periodMs;

  // Calculate the Duty Cycle (fill factor)
  // Formula: (pulseWidthMs / periodMs) * (2^resolution - 1)

  uint32_t duty = (pulseWidthMs * ((1 << pulseResolution) - 1)) / periodMs;

  // Apply the settings
  ledcSetup(ledcChannel, frequency, pulseResolution);
  ledcWrite(ledcChannel, duty);
}

void setup() {
  Serial.begin(115200);

  // Attach pins to PWM channel

  ledcAttachPin(outputPin, ledcChannel);
  ledcAttachPin(ledPin, ledcChannel);

  updatePWM(currentFreq, currentDutyPercent);

  Serial.println("---------------------------------------");
  Serial.println("Generator is ready for use.");
  Serial.println("To set PWM: <frequency> <duty%>");
  Serial.println("Example: 10000 50 (for 10 kHz at 50% duty cycle)");
  Serial.println("To set PULSE: <period_ms> <pulse_duty_ms>");
  Serial.println("---------------------------------------");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');

    char mode[10];  // buffer for prefix (PWM or PULSE)
    uint32_t f;
    int p;

    // parsing string with sscanf, expecting format: "PWM 10000 50" or "PULSE 1000 500"

    int found = sscanf(input.c_str(), "%[^:]: %u %d", mode, &f, &p);
    Serial.printf("Set PULSE: Period = %u ms, Pulse Width = %u ms\n", f, p);

    if (found == 3) {
      if (strcmp(mode, "PWM") == 0) {
        // pwm logic
        if (f >= 5000 && f <= 500000 && p >= 0 && p <= 100) {
          currentFreq = f;
          currentDutyPercent = p;
          updatePWM(currentFreq, currentDutyPercent);

        } else {
          Serial.printf("Set Error: Period = %u ms, Pulse Width = %u ms\n", f, p);
          Serial.println("Set error: Frequency 5000-500000 Hz, Percentages 0-100%");
        }
      } else if (strcmp(mode, "PULSE") == 0) {
        // PULSE logic

        if (f >= 0 && f <= 3000 && p >= 0 && p <= 3000) {
          setPulse(p, f);

        } else {
          Serial.println("ERROR: Period 10-3000 ms, Pulse Width 0-1000 ms");
          Serial.printf("Set Error: Period = %u ms, Pulse Width = %u ms\n", f, p);
        }
      }
    } else {
      Serial.println("Invalid input format. Use: PWM <frequency> <duty%> or PULSE <period_ms> <pulse_duty_ms>");
    }
  }
}
