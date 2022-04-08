#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdio.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_CAP1188.h>

TFT_eSPI tft = TFT_eSPI();
char buf[6];

// Reset Pin is used for I2C or SPI
#define CAP1188_RESET  9

// CS pin is used for software or hardware SPI
#define CAP1188_CS  10

// These are defined for software SPI, for hardware SPI, check your 
// board's SPI pins in the Arduino documentation
#define CAP1188_MOSI  11
#define CAP1188_MISO  12
#define CAP1188_CLK  13

// For I2C, connect SDA to your Arduino's SDA pin, SCL to SCL pin
// On UNO/Duemilanove/etc, SDA == Analog 4, SCL == Analog 5
// On Leonardo/Micro, SDA == Digital 2, SCL == Digital 3
// On Mega/ADK/Due, SDA == Digital 20, SCL == Digital 21

// Use I2C, no reset pin!
Adafruit_CAP1188 cap = Adafruit_CAP1188();

// Or...Use I2C, with reset pin
//Adafruit_CAP1188 cap = Adafruit_CAP1188(CAP1188_RESET);

// Or... Hardware SPI, CS pin & reset pin 
// Adafruit_CAP1188 cap = Adafruit_CAP1188(CAP1188_CS, CAP1188_RESET);

// Or.. Software SPI: clock, miso, mosi, cs, reset
//Adafruit_CAP1188 cap = Adafruit_CAP1188(CAP1188_CLK, CAP1188_MISO, CAP1188_MOSI, CAP1188_CS, CAP1188_RESET);

void setup() {
  // Serial console
  Serial.begin(9600);

  // Display and Touchscreen
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(0x0000);
  tft.setTextSize(5);

  // Touch Sensor (I2C)
  Serial.println("CAP1188 test!");
  if (!cap.begin()) {
    Serial.println("CAP1188 not found");
    while (1);
  }
  Serial.println("CAP1188 found!");

  // Touch Sensor (SPI)


  // Buttons



}

void loop() {
  uint8_t total = 0;

  // Touch sensor
  uint8_t touched = cap.touched();
  if (touched) {
    for (uint8_t i=0; i<8; i++) {
      if (touched & (1 << i)) {
        Serial.print("C"); Serial.print(i+1); Serial.print("\t");

        total += 1;
      }
    }
    Serial.println();
  }

  // Touchscreen

  // Buttons

  // Display total to screen
  sprintf(buf, "%d", total);
  tft.drawString(buf, 50, 0 , 2);
  //tft.setTextDatum(TC_DATUM);
  //tft.setTextColor(0xFFFF);
  
  delay(100);
}
