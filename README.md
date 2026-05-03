# VenoSense: Real-Time Hemodynamic & Immobility Monitor 🩺

**1st Place Winner** | Dean’s Imagineering Competition | Singh Lab (Sophomore Category)

---

## 🚀 Overview
**VenoSense** is an integrated health monitoring solution designed to detect physiological precursors to Deep Vein Thrombosis (DVT). By combining heart rate variability, localized swelling detection, and movement tracking, VenoSense provides a real-time "Stability Score" to help patients and providers monitor vascular health.

## 🛠 Hardware Architecture
The system utilizes a dual-processor architecture to isolate sensitive analog signal processing from high-bandwidth Bluetooth communications.

* **Arduino Uno (Signal Core):** * **Heart Rate:** Processes high-frequency IR sensor data for pulse detection.
    * **Stretch Sensing:** Monitors limb circumference via a resistive stretch sensor.
    * **Calibration:** Implements a 30-second "True Baseline" acquisition phase.
* **Adafruit Feather ESP32-S3 (Communication Core):** * **BLE Stack:** Manages a custom GATT service for high-throughput data transmission.
    * **BPM Averaging:** Calculates a 30-second sliding window average using a circular buffer.
    * **IMU Tracking:** Monitors patient movement via an onboard LIS3DH accelerometer.
* **The Bridge:** Hardware-level logic isolation using a voltage divider to ensure 5V (Uno) to 3.3V (Feather) signal integrity.

## 📡 Sensor Intelligence

### 1. Adaptive Heart Rate Detection
VenoSense doesn't just count beats; it analyzes them.
* **Digital Triggering:** The Uno detects the physical pulse and sends a 20ms hardware interrupt to the Feather.
* **Sliding Window:** The Feather stores the last 60 heartbeats in a circular buffer, calculating a stable BPM over a 30-second window to filter out noise and motion artifacts.

### 2. Intelligent Stretch (Swelling) Sensing
To detect gradual swelling while ignoring minor sensor drift, VenoSense uses a **Fixed Baseline** method:
* **30s Calibration:** On startup, the system locks in the user's "still" state.
* **Hysteresis Logic:** * **Trigger:** 20% drop from baseline (Swelling Detected).
    * **Reset:** 10% recovery (Status Cleared).
    * *This prevents "flickering" alerts and ensures only significant swelling is reported.*

### 3. Immobility & Stability Tracking
Using the LIS3DH accelerometer, the system computes the total displacement vector ($|ΔX| + |ΔY| + |ΔZ|$). 
* **Motion Awareness:** If movement is detected, the UI warns the user to hold still for accurate reading.
* **Resting Score:** Accumulates "stability points" when the patient is safely immobile but active enough to maintain circulation.

## Software Stack
* **Firmware:** C++/Arduino (Logic-level isolation & State Machine).
* **Backend:** Qt/C++ (Asynchronous Bluetooth LE Controller).
* **Frontend:** QML / Qt Quick.
    * **Dynamic UI:** Real-time StepLineSeries graphs for digital pulse/stretch data.
    * **Safety UX:** Modal calibration overlays to ensure data integrity during baseline capture.

---

## Project Status

Complete prototype — awarded **1st Place (Sophomore Division)** at the Dean's Imagineering Competition, Singh Lab.

---

## License

MIT License — see `LICENSE` for details.