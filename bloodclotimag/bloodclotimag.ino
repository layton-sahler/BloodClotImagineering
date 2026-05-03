#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

Adafruit_LIS3DH lis = Adafruit_LIS3DH();
BLECharacteristic *pCharacteristic;

const int stretchInputPin = 12;  //to 9
const int  hrInputPin = 9; //to 13
bool lastHRState = LOW;

const int MAX_BEATS = 60;
unsigned long beatTimes[MAX_BEATS];
int beatIndex = 0;
int totalBeatsStored = 0;
float stableBPM = 0;
bool deviceConnected = false;
String latestPulse = "0";

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; };
    void onDisconnect(BLEServer* pServer) { 
      deviceConnected = false; 
      BLEDevice::startAdvertising();
    }
};

void setup() {
    delay(100);
    Serial.begin(115200);
    pinMode(hrInputPin, INPUT);
    pinMode(stretchInputPin, INPUT);
    
    Wire.begin(3, 4);

    if (!lis.begin(0x18)) {
        Serial.println("0x18 failed, trying 0x19...");
        if (!lis.begin(0x19)) {
            Serial.println("ERROR: LIS3DH not found on either address!");
            while(1);
        }
    }

    BLEDevice::init("CLOT_SENSOR_V3");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();
    BLEDevice::startAdvertising();
}

void loop() {
    unsigned long now = millis();

    // 1. Read Stretch Status (Pin 11)
    int stretchStatus = digitalRead(stretchInputPin);

    // 2. Read HR Pulse (Pin 10) and calc BPM
    bool currentHRState = digitalRead(hrInputPin);
    if (currentHRState == HIGH && lastHRState == LOW) {
        static unsigned long lastBeatMillis = 0;
        if (now - lastBeatMillis > 300) {
            latestPulse = "1";
            beatTimes[beatIndex] = now;
            beatIndex = (beatIndex + 1) % MAX_BEATS;
            if (totalBeatsStored < MAX_BEATS) totalBeatsStored++;

            int count = 0;
            unsigned long oldestBeat = now;
            for (int i = 0; i < totalBeatsStored; i++) {
                if (now - beatTimes[i] < 30000) {
                    count++;
                    if (beatTimes[i] < oldestBeat) oldestBeat = beatTimes[i];
                }
            }
            if (count > 1) {
                float timeSpanMinutes = (now - oldestBeat) / 60000.0;
                stableBPM = (count - 1) / timeSpanMinutes;
            }
            lastBeatMillis = now;
        }
    }
    lastHRState = currentHRState;

    if (deviceConnected) {
        sensors_event_t event;
        lis.getEvent(&event);

        // Data: Stretch(0/1), AccelX, AccelY, AccelZ, Pulse(0/1), BPM
        String packet = String(stretchStatus) + "," + 
                        String(event.acceleration.x, 2) + "," + 
                        String(event.acceleration.y, 2) + "," + 
                        String(event.acceleration.z, 2) + "," +
                        latestPulse + "," + 
                        String(stableBPM, 1); 
        
        pCharacteristic->setValue(packet.c_str());
        pCharacteristic->notify();
        Serial.println(packet);
        latestPulse = "0";
        delay(20); 
    } else {
        delay(50);
    }
}