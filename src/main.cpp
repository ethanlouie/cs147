#include <Arduino.h>
#include <stdio.h>
float old_desired_temp = 0;
float old_degreesF = 0;

/*
 * LED
 * Buttons
 *
 * Fan State
 *   0 = Auto
 *   1 = Always On
 *
 * System State
 *   0 = No Temp Adjustment
 *   1 = Manual Temp
 *   2 = Weather Predictive Temp
 *
 * Temp State
 *   0 = On Target
 *   1 = Needs Cooling (too hot)
 *   2 = Needs Heat (too cold)
 */
#include "Button2.h"
#define HEAT_PIN 12
#define COOL_PIN 13
#define FAN_PIN 15
#define BUTTON1_PIN 0
#define BUTTON2_PIN 35
Button2 button1;
Button2 button2;
int fan_state = 0;
int system_state = 1;
int temp_state = 0;

void manageModeState(void *parameter)
{
  // vTaskCoreAffinitySet(NULL, (1 << 1));

  button1.begin(BUTTON1_PIN);
  button2.begin(BUTTON2_PIN);

  pinMode(HEAT_PIN, OUTPUT);
  pinMode(COOL_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  for (;;)
  {
    button1.loop();
    if (button1.wasPressed())
    {
      system_state = (system_state + 1) % 3;
      old_desired_temp = -1;
      button1.read();
    }
    button2.loop();
    if (button2.wasPressed())
    {
      fan_state = !fan_state;
      old_degreesF = -1;
      button2.read();
    }

    /* Set energy state based upon fan and system states */
    if (system_state)
    {
      if (temp_state == 0)
      {
        digitalWrite(HEAT_PIN, LOW);
        digitalWrite(COOL_PIN, LOW);
      }
      else if (temp_state == 1)
      {
        digitalWrite(HEAT_PIN, LOW);
        digitalWrite(COOL_PIN, HIGH);
      }
      else if (temp_state == 2)
      {
        digitalWrite(HEAT_PIN, HIGH);
        digitalWrite(COOL_PIN, LOW);
      }
    }
    else
    {
      digitalWrite(HEAT_PIN, LOW);
      digitalWrite(COOL_PIN, LOW);
    }

    digitalWrite(FAN_PIN, (system_state && temp_state) || fan_state);

    vTaskDelay(100);
  }
}

/*
 * Tempurature
 * Humidity
 */
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;
sensors_event_t humidity_event, temp_event;
float degreesF = 0;
float humidity = 0;

void fetchTempHumidity(void *parameter)
{
  // vTaskCoreAffinitySet(NULL, (1 << 1));

  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      vTaskDelay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  for (;;)
  {
    aht.getEvent(&humidity_event, &temp_event);
    degreesF = 32 + (temp_event.temperature * 1.8);
    humidity = humidity_event.relative_humidity;

    vTaskDelay(500);
  }
}

/*
 * Display
 * Potentiometer
 */
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
TFT_eSPI tft = TFT_eSPI();
#define POTENTIOMETER_PIN 36

int MIN_TEMP = 65;
int MAX_TEMP = 90;
int center_x = 119;     // x center of the knob
int center_y = 250;     // y center of the knob (outside of the screen)
int radius_pixel = 215; // radius for pixel tickmarks
int radius_line = 210;  // radius for medium tickmark
int RADIUS_LARGE = 205; // radius for large tickmark
int radius_text = 205;  // radius for text

float desired_temp = 0;
int old_potentiometer_value = 0;
int raw_potentiometer = 0;
int potentiometer_value = 0;            // value from the potentiometer
char buffer[20];                        // helper buffer for converting values into C-style string (array of chars)
int string_width;                       // helper value for string widths
int string_height;                      // helper value for string heights
float pixel_x = 0;                      // x pos for pixel
float pixel_y = 0;                      // y pos for pixel
float line_x = 0;                       // x pos for line end
float line_y = 0;                       // y pos for line end
float text_x = 0;                       // x pos for text
float text_y = 0;                       // y pos for text
int angle;                              // angle for the individual tickmarks
int tick_value;                         // numeric value for the individual tickmarks
byte precalculated_x_radius_pixel[180]; // lookup table to prevent expensive sin/cos calculations
byte precalculated_y_radius_pixel[180]; // lookup table to prevent expensive sin/cos calculations

void showDisplay(void *parameter)
{
  // vTaskCoreAffinitySet(NULL, (1 << 1));

  pinMode(POTENTIOMETER_PIN, INPUT);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  for (int i = 0; i < 180; i++)
  { // pre-calculate x and y positions into the look-up tables
    precalculated_x_radius_pixel[i] = sin(radians(i - 90)) * radius_pixel + center_x;
    precalculated_y_radius_pixel[i] = -cos(radians(i - 90)) * radius_pixel + center_y;
  }

  for (;;)
  {
    // read potentiometer with exponentially-weighted smoothing
    raw_potentiometer += (analogRead(POTENTIOMETER_PIN) - raw_potentiometer) / 4;

    // map to percentage and tempurature
    potentiometer_value = map(raw_potentiometer, 0, 4095, 0, 1000);
    desired_temp = MIN_TEMP + potentiometer_value / 1000.0 * (MAX_TEMP - MIN_TEMP);

    // if predictive weather control active, replace potentiometer input with web server response
    if (system_state == 2)
    {
      desired_temp = 69; // put web response here
      potentiometer_value = map(desired_temp, MIN_TEMP, MAX_TEMP, 0, 1000);
    }

    // Set tempurature state
    if (desired_temp < (degreesF - 1))
    {
      temp_state = 1;
    }
    else if (desired_temp > (degreesF + 1))
    {
      temp_state = 2;
    }
    else
    {
      temp_state = 0;
    }

    /* Top Desired Temp */
    tft.setTextSize(3);
    string_height = tft.fontHeight();
    if ((int)old_desired_temp != (int)desired_temp)
    {
      old_desired_temp = desired_temp;

      sprintf(buffer, "%.0f%s", desired_temp, " F");
      string_width = tft.textWidth(buffer); // calculate string width

      tft.fillRoundRect((240 - (string_width + 4)) / 2, 0,
                        string_width + 4, string_height + 4,
                        1, TFT_WHITE); // draw background rounded rectangle
      tft.fillTriangle(119 - 4, string_height,
                       119 + 4, string_height,
                       119, string_height + 8,
                       TFT_WHITE);                          // draw small arrow below the rectangle
      tft.drawCircle(130, string_height / 4, 4, TFT_BLACK); // degrees symbol
      tft.drawCircle(130, string_height / 4, 3, TFT_BLACK); // degrees symbol

      tft.setTextColor(TFT_BLACK);                         // set color to black
      tft.drawString(buffer, (240 - string_width) / 2, 2); // draw the value on top of the display

      // system state
      if (system_state == 0)
      {
        sprintf(buffer, "%s", "OFF");
      }
      else if (system_state == 1)
      {
        sprintf(buffer, "%s", "MANUAL");
      }
      else if (system_state == 2)
      {
        sprintf(buffer, "%s", "PREDICTIVE");
      }
      tft.setTextColor(TFT_WHITE); // set color to black
      tft.setTextSize(1);
      string_height = tft.fontHeight();
      tft.fillRect(0, 0, string_width + 2, string_height + 2, TFT_BLACK);
      tft.drawString(buffer, 2, 2);
    }

    tft.setTextSize(3);
    string_height = tft.fontHeight();
    /* Bottom Current Temp */
    if ((int)(old_degreesF * 10) != (int)(degreesF * 10))
    {
      old_degreesF = degreesF;

      sprintf(buffer, "%.1f%s", degreesF, " F");
      string_width = tft.textWidth(buffer); // calculate string width

      tft.fillRect(0, 133 - string_height, 240, string_height, TFT_BLACK); // partially clear screen
      tft.drawCircle(145, 138 - string_height, 4, TFT_WHITE);              // degrees symbol
      tft.drawCircle(145, 138 - string_height, 3, TFT_WHITE);              // degrees symbol

      tft.drawString(buffer, (240 - string_width) / 2, 133 - string_height); // draw the value on bottom of the display

      // Fan indicator
      if (fan_state)
      {
        sprintf(buffer, "%s", "FAN ON");
      }
      else
      {
        sprintf(buffer, "%s", "AUTO");
      }
      tft.setTextSize(1);
      string_height = tft.fontHeight();
      tft.drawString(buffer, 2, 133 - string_height);
    }

    tft.setTextSize(3);
    string_height = tft.fontHeight();
    /* Tickmarks and Numbers */
    tft.fillRect(0, string_height + 9, 240, 126 - (string_height + 2) * 2, TFT_BLACK); // partially clear screen
    tft.setTextColor(TFT_WHITE);                                                       // set color to black
    tft.setTextSize(2);                                                                // set smaller font for tickmarks

    for (int i = -35; i <= 35; i = i + 3)
    {                                                                 // only try to calculate tickmarks that would end up be displayed
      angle = i + ((potentiometer_value * 3) / 10) % 3;               // final angle for the tickmark
      tick_value = round((potentiometer_value / 10.0) + angle / 3.0); // get number value for each tickmark

      pixel_x = precalculated_x_radius_pixel[angle + 90]; // get x value from lookup table
      pixel_y = precalculated_y_radius_pixel[angle + 90]; // get y value from lookup table

      if (pixel_x < 0 || pixel_x > 240 || pixel_y < 0 || pixel_y > 135 || tick_value < 0 || tick_value > 100)
        continue; //  only draw pixels inside of the screen, and tickmarks between 0-100%

      if (tick_value % (100 / (MAX_TEMP - MIN_TEMP)) == 0)
      {
        tick_value = map(tick_value, 0, 100, MIN_TEMP, MAX_TEMP); // change to degrees

        // draw numbers
        text_x = sin(radians(angle)) * radius_text + center_x;     // calculate x pos for the text
        text_y = -cos(radians(angle)) * radius_text + center_y;    // calculate y pos for the text
        itoa(tick_value, buffer, 10);                              // convert integer to string
        string_width = tft.textWidth(buffer);                      // get string width
        tft.drawString(buffer, text_x - string_width / 2, text_y); // draw text - tickmark value

        // draw medium tickmark
        line_x = sin(radians(angle)) * (radius_line) + center_x;   // calculate x pos for the line end
        line_y = -cos(radians(angle)) * radius_line + center_y;    // calculate y pos for the line end
        tft.drawLine(pixel_x, pixel_y, line_x, line_y, TFT_WHITE); // draw the line
      }
      else
      {                                             // draw small tickmark == pixel tickmark
        tft.drawPixel(pixel_x, pixel_y, TFT_WHITE); // draw a single pixel
      }
    }

    vTaskDelay(15);
  }
}

/*
 * Wifi
 *
 * POST request containing
 *   tempurature
 *   humidity
 *   Energy Usage of system = (system_state && (temp_state + bool(temp_state))) + (fan_state && !temp_state)
 *     0 = No Power Usage
 *     1 = Cooling
 *     2 = Heating
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
char kPath[50];
char ip[] = "169.234.5.83";
int state = -1;
WiFiMulti wifiMulti;

void wifiUpload(void *parameter)
{
  // vTaskCoreAffinitySet(NULL, (1 << 0));

  wifiMulti.addAP("Pixel_7807", "ethanalllowercase");
  wifiMulti.addAP("UCInet Mobile Access", "");
  wifiMulti.addAP("Packet_Dispensary", "passwordalllowercase");

  for (;;)
  {
    state = bool(system_state) * temp_state;

    // wait for WiFi connection
    if ((wifiMulti.run() == WL_CONNECTED))
    {
      HTTPClient http;

      Serial.print("[HTTP] begin...\n");
      // configure traged server and url
      sprintf(kPath, "http://%s:5000/data?temp=%.1f&humidity=%.2f&state=%i", ip, degreesF, humidity, state);
      Serial.print(kPath);

      http.begin(kPath); // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (true)
        {
          String payload = http.getString();
          Serial.println(payload);
        }
      }
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
    // taskEXIT_CRITICAL();

    vTaskDelay(5000);
  }
}

/*
 * Setup
 */
void setup()
{
  Serial.begin(9200);

  xTaskCreate(
      fetchTempHumidity,    // Function that should be called
      "temp/humidity task", // Name of the task (for debugging)
      1500,                 // Stack size (bytes)
      NULL,                 // Parameter to pass
      2,                    // Task priority
      NULL                  // Task handle
  );

  xTaskCreate(
      manageModeState,                 // Function that should be called
      "manage system mode/state task", // Name of the task (for debugging)
      1000,                            // Stack size (bytes)
      NULL,                            // Parameter to pass
      3,                               // Task priority
      NULL                             // Task handle
  );

  xTaskCreate(
      showDisplay,    // Function that should be called
      "display task", // Name of the task (for debugging)
      2000,           // Stack size (bytes)
      NULL,           // Parameter to pass
      9,              // Task priority
      NULL            // Task handle
  );

  xTaskCreate(
      wifiUpload,  // Function that should be called
      "wifi task", // Name of the task (for debugging)
      3000,        // Stack size (bytes)
      NULL,        // Parameter to pass
      15,          // Task priority
      NULL         // Task handle
  );
   vTaskDelete(NULL);
}

void loop() { }