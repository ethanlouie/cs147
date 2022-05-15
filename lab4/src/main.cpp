#include <Arduino.h>
#include "Wire.h"
#include "SPI.h"
#define RED_PIN 12

#include "SparkFunLSM6DSO.h"
LSM6DSO myIMU; // Default constructor is I2C, addr 0x6B
bool z_pos = true;

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

int stepCount = 0;
BLECharacteristic *pCharacteristic = NULL;
char buf[10];

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value == "ledon")
    {
      digitalWrite(RED_PIN, HIGH);
    }
    if (value == "ledoff")
    {
      digitalWrite(RED_PIN, LOW);
    }

    if (value.length() > 0)
    {
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++)
        Serial.print(value[i]);

      Serial.println();
      Serial.println("*********");
    }
  }

  void onRead(BLECharacteristic *pCharacteristic)
  {
    Serial.println("READ FROM BLE");

    sprintf(buf, "steps: %d", stepCount);
    pCharacteristic->setValue(buf);

    // pCharacteristic->notify();
  }
};

void setup()
{
  Serial.begin(9200);
  delay(500);

  // Red LED
  pinMode(RED_PIN, OUTPUT);

  // Gyroscope
  Wire.begin();
  delay(10);
  if (myIMU.begin())
    Serial.println("Ready.");
  else
  {
    Serial.println("Could not connect to IMU.");
    Serial.println("Freezing");
  }
  if (myIMU.initialize(BASIC_SETTINGS))
    Serial.println("Loaded Settings.");

  // Bluetooth
  BLEDevice::init("Group19");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  float z = myIMU.readFloatGyroZ();
  Serial.println(z);
  // Serial.println(abs(z));
  // if values greater than abs(5), increment



  if (abs(z) > 5 && ((z > 0 && !z_pos) || (z < 0 && z_pos)))
  {
    stepCount++;
    Serial.printf("Step Count: %d\n", stepCount);
    z_pos = !z_pos;
  }

  delay(500);
}
