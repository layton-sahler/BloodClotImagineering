#include <Wire.h>

const int digitalOutPin = 9; 
const int heartbeatOut = 13; 
const int stretchPin = A3;
const int hrPin = A0;

// Heart Rate Variables
int baseline = 330;
int sensitivity = 25;
int lastValue = 330;
int waitms = 500;
unsigned long detectiontime = 0;

// --- NEW Calibration & Stretch Variables ---
float fixedBaseline = 0;
unsigned long calibrationStart = 0;
const unsigned long calibrationDuration = 30000; // 30 seconds
unsigned long sampleCount = 0;
float runningSum = 0;
bool isCalibrated = false;

// Hysteresis to prevent flickering
bool isTriggered = false;

void setup() {
  Serial.begin(115200);
  pinMode(heartbeatOut, OUTPUT);
  pinMode(digitalOutPin, OUTPUT);
  digitalWrite(heartbeatOut, LOW);
  digitalWrite(digitalOutPin, LOW);

  calibrationStart = millis(); // Start the 30s clock
}

void loop() {
  unsigned long currtime = millis();

  // --- 1. HEART RATE LOGIC (Consistent with your original) ---
  int hrRaw = analogRead(hrPin);
  int smoothed = (hrRaw * 0.2) + (lastValue * 0.8);
  lastValue = smoothed;

  int cleanSignal = (abs(smoothed - baseline) < sensitivity) ? baseline : smoothed;
  unsigned long timeSinceLast = currtime - detectiontime;

  if (cleanSignal != baseline && timeSinceLast > waitms) {
    detectiontime = currtime;
    digitalWrite(heartbeatOut, HIGH); 
    delay(20); 
    digitalWrite(heartbeatOut, LOW);
  }

  // --- 2. STRETCH LOGIC (Fixed Baseline Version) ---
  int sVal = analogRead(stretchPin);

  if (!isCalibrated) {
    // PHASE A: CALIBRATING (First 30 Seconds)
    runningSum += sVal;
    sampleCount++;

    if (currtime - calibrationStart >= calibrationDuration) {
      fixedBaseline = runningSum / (float)sampleCount;
      isCalibrated = true;
    }
    
    // Safety: Keep output LOW during calibration
    digitalWrite(digitalOutPin, LOW);

  } else {
    // PHASE B: MONITORING (After 30 Seconds)
    
    // We use your 80% threshold, but against the LOCKED baseline
    float triggerThreshold = fixedBaseline * 0.95; 
    // Reset at 90% to provide hysteresis (prevents jitter)
    float resetThreshold = fixedBaseline * 0.95; 

    if (!isTriggered) {
      if (sVal < triggerThreshold) {
        digitalWrite(digitalOutPin, HIGH);
        isTriggered = true;
      }
    } else {
      if (sVal > resetThreshold) {
        digitalWrite(digitalOutPin, LOW);
        isTriggered = false;
      }
    }
  }

  // --- 3. SERIAL OUTPUT ---
  Serial.print("HR:"); Serial.print(hrRaw);
  Serial.print(",Stretch:"); Serial.print(sVal);
  
  if (!isCalibrated) {
    Serial.println(",Status:CALIBRATING");
  } else {
    Serial.print(",Baseline:"); Serial.print(fixedBaseline);
    Serial.print(",Trigger:"); Serial.println(isTriggered ? "1" : "0");
  }
  
  delay(10);
}