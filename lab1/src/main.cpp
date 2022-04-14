#include <Arduino.h>
#include <stdio.h>
#include <Wire.h>
#include <SPI.h>
uint8_t total;

/* Display */
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
char buf[3] = "-1";

/* Touch Sensor
 * I2C: connect SDA to pin 21, SCL to pin 22
 * Software SPI: clock, miso, mosi, cs
 */
#include <Adafruit_CAP1188.h>
#define CAP1188_CLK 25
#define CAP1188_MISO 33
#define CAP1188_MOSI 32
#define CAP1188_CS 26
#define CAP1188_RESET -1 // Not used
// Adafruit_CAP1188 cap = Adafruit_CAP1188(); // I2C
Adafruit_CAP1188 cap = Adafruit_CAP1188(CAP1188_CLK, CAP1188_MISO, CAP1188_MOSI, CAP1188_CS, CAP1188_RESET); // SPI

/* Buttons */
#include "Button2.h"
#define BUTTONL_PIN 0
#define BUTTONR_PIN 35
Button2 buttonL;
Button2 buttonR;

void setup()
{
  /* Serial console */
  Serial.begin(9600);

  /* Display*/
  tft.init();
  tft.setTextSize(7);
  tft.setTextDatum(TL_DATUM);
  tft.setRotation(3);

  /* Touch Sensor */
  Serial.println("CAP1188 test!");
  if (!cap.begin())
  {
    Serial.println("CAP1188 not found");
    while (1)
      ;
  }
  Serial.println("CAP1188 found!");

  /* Buttons */
  buttonL.begin(BUTTONL_PIN);
  buttonR.begin(BUTTONR_PIN);
}

void loop()
{
  total = 0;

  /* Touch Sensor */
  uint8_t touched = cap.touched();
  if (touched)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if (touched & (1 << i))
      {
        // Serial.print("C"); Serial.print(i+1); Serial.print("\t");
        total += 1;
      }
    }
    // Serial.println();
  }

  /* Buttons */
  buttonL.loop();
  buttonR.loop();
  if (buttonL.isPressed() || buttonR.isPressed())
  {
    total += buttonL.isPressed() + buttonR.isPressed();
    buttonL.read();
    buttonR.read();
  }

  /* Display Total to Screen */
  if (atoi(buf) != total)
  {
    sprintf(buf, "%d", total);
    tft.fillScreen(TFT_BLACK);
    tft.drawString(buf, 50, 0, 2);
    delay(100);
  }
}
