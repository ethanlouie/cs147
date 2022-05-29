#include <Arduino.h>
#include <stdio.h>
#define RED_PIN 12
#define BLUE_PIN 13
#define GREEN_PIN 15
#define POTENTIOMETER_PIN 36

#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;
float degreesF = 0;
float humidity = 0;

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
TFT_eSPI tft = TFT_eSPI();

#define MIN_TEMP 65
#define MAX_TEMP 85
int desired_temp = 72;
int old_desired_temp = 0;
int old_potentiometer_value = 0;
int raw_potentiometer = 0;
int potentiometer_value = 0; // value from the potentiometer
char buffer[20];             // helper buffer for converting values into C-style string (array of chars)
int string_width;            // helper value for string widths
int string_height;

float pixel_x = 0; // x pos for pixel
float pixel_y = 0; // y pos for pixel
float line_x = 0;  // x pos for line end
float line_y = 0;  // y pos for line end
float text_x = 0;  // x pos for text
float text_y = 0;  // y pos for text

int center_x = 119;     // x center of the knob
int center_y = 150;     // 200;     // y center of the knob (outside of the screen)
int radius_pixel = 115; // radius for pixel tickmarks
int radius_line = 110;  // radius for medium tickmark
int RADIUS_LARGE = 105; // radius for large tickmark
int radius_text = 90;   // radius for text

int angle;      // angle for the individual tickmarks
int tick_value; // numeric value for the individual tickmarks

byte precalculated_x_radius_pixel[180]; // lookup table to prevent expensive sin/cos calculations
byte precalculated_y_radius_pixel[180]; // lookup table to prevent expensive sin/cos calculations

#include <WiFi.h>
#include <HttpClient.h>
WiFiClient c;
HttpClient http(c);

char ssid[] = "Packet_Dispensary";    // your network SSID (name)
char pass[] = "passwordalllowercase"; // your network password (use for WPA, or use as key for WEP)
const char kHostname[] = "192.168.1.14";//13.57.25.227";
char tempStr[] = "/?temp=";
char humidStr[] = "&humidity=";

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void wifiUpload(void *parameter)
{
  for (;;)
  {

    // print temp/humidity to console
    sensors_event_t humidity_event, temp_event;
    aht.getEvent(&humidity_event, &temp_event); // populate temp and humidity objects with fresh data
    degreesF = 32 + (temp_event.temperature * 1.8);
    humidity = humidity_event.relative_humidity;

    Serial.print("Temperature: ");
    Serial.print(degreesF);
    Serial.println(" degrees C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("% rH");

    /* WIFI THINGS */
    // format kPath
    char kPath[50];
    sprintf(kPath, "/?temp=%.1f&humidity=%f", degreesF, humidity);
    Serial.print(kPath);

    int err = 0;
    err = http.get(kHostname, 5000, kPath);
    if (err == 0)
    {
      Serial.println("startedRequest ok");

      err = http.responseStatusCode();
      if (err >= 0)
      {
        Serial.print("Got status code: ");
        Serial.println(err);

        // Usually you'd check that the response code is 200 or a
        // similar "success" code (200-299) before carrying on,
        // but we'll print out whatever response we get

        err = http.skipResponseHeaders();
        if (err >= 0)
        {
          int bodyLen = http.contentLength();
          Serial.print("Content length is: ");
          Serial.println(bodyLen);
          Serial.println();
          Serial.println("Body returned follows:");

          // Now we've got to the body, so we can print it out
          unsigned long timeoutStart = millis();
          char c;
          // Whilst we haven't timed out & haven't reached the end of the body
          while ((http.connected() || http.available()) &&
                 ((millis() - timeoutStart) < kNetworkTimeout))
          {
            if (http.available())
            {
              c = http.read();
              // Print out this character
              Serial.print(c);

              bodyLen--;
              // We read something, reset the timeout counter
              timeoutStart = millis();
            }
            else
            {
              // We haven't got any data, so let's pause to allow some to
              // arrive
              delay(kNetworkDelay);
            }
          }
        }
        else
        {
          Serial.print("Failed to skip response headers: ");
          Serial.println(err);
        }
      }
      else
      {
        Serial.print("Getting response failed: ");
        Serial.println(err);
      }
    }
    else
    {
      Serial.print("Connect failed: ");
      Serial.println(err);
    }

    http.stop();
    vTaskDelay(1000);
  }
}

void setup()
{
  /* LED */
  Serial.begin(9200);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  /* Temp Humidity */
  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  /* Display */
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  for (int i = 0; i < 180; i++)
  { // pre-calculate x and y positions into the look-up tables
    precalculated_x_radius_pixel[i] = sin(radians(i - 90)) * radius_pixel + center_x;
    precalculated_y_radius_pixel[i] = -cos(radians(i - 90)) * radius_pixel + center_y;
  }

  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  xTaskCreate(
      wifiUpload, // Function that should be called
      "wifi",     // Name of the task (for debugging)
      3000,       // Stack size (bytes)
      NULL,       // Parameter to pass
      1,          // Task priority
      NULL        // Task handle
  );
}

void loop()
{

  /* Potentiometer serial print*/
  float var = analogRead(POTENTIOMETER_PIN);
  Serial.println(var);

  /* DISPLAY */
  raw_potentiometer = analogRead(POTENTIOMETER_PIN);

  // prevent jitter of small changes
  int delta = abs(raw_potentiometer - old_potentiometer_value);
  if (delta < 40)
  {
    raw_potentiometer = old_potentiometer_value + (delta > 0) - (delta < 0);
  }
  old_potentiometer_value = raw_potentiometer;

  // map to percentage and tempurature
  potentiometer_value = map(raw_potentiometer, 0, 4095, 0, 1000);
  desired_temp = MIN_TEMP + floor(potentiometer_value / 1000.0 * (MAX_TEMP - MIN_TEMP));

  /* Draw the desired temp on top */
  if (old_desired_temp != desired_temp)
  {
    old_desired_temp = desired_temp;

    tft.setTextSize(3);
    string_height = tft.fontHeight();
    sprintf(buffer, "%i%s", desired_temp, " F");
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
  }

  /* Draw tickmarks */

  tft.fillRect(0, string_height + 9, 240, 135, TFT_BLACK);          // partially clear screen
  tft.setTextColor(TFT_WHITE);                                      // set color to black
  tft.setTextSize(2);                                               // set smaller font for tickmarks
  for (int i = -80; i <= 80; i = i + 3)                             /////////////////// EDIT THIS LATER /////////////////////////////////////////
  {                                                                 // only try to calculate tickmarks that would end up be displayed
    angle = i + ((potentiometer_value * 3) / 10) % 3;               // final angle for the tickmark
    tick_value = round((potentiometer_value / 10.0) + angle / 3.0); // get number value for each tickmark

    pixel_x = precalculated_x_radius_pixel[angle + 90]; // get x value from lookup table
    pixel_y = precalculated_y_radius_pixel[angle + 90]; // get y value from lookup table

    if (pixel_x < 0 || pixel_x > 240 || pixel_y < 0 || pixel_y > 135 || tick_value < 0 || tick_value > 100)
      continue; // only draw pixels inside of the screen, and tickmarks between 0-100%

    if (tick_value % (100 / (MAX_TEMP - MIN_TEMP)) == 0)
    {
      tick_value = map(tick_value, 0, 100, MIN_TEMP, MAX_TEMP); // change to degrees
      if (tick_value == MIN_TEMP || tick_value == MAX_TEMP || tick_value % 5 == 0)
      {
        // draw text
        text_x = sin(radians(angle)) * radius_text + center_x;     // calculate x pos for the text
        text_y = -cos(radians(angle)) * radius_text + center_y;    // calculate y pos for the text
        itoa(tick_value, buffer, 10);                              // convert integer to string
        string_width = tft.textWidth(buffer);                      // get string width
        tft.drawString(buffer, text_x - string_width / 2, text_y); // draw text - tickmark value

        // draw large tickmark
        line_x = sin(radians(angle)) * RADIUS_LARGE + center_x;    // calculate x pos for the line end
        line_y = -cos(radians(angle)) * RADIUS_LARGE + center_y;   // calculate y pos for the line end
        tft.drawLine(pixel_x, pixel_y, line_x, line_y, TFT_WHITE); // draw the line
      }
      else
      {
        // draw medium tickmark
        line_x = sin(radians(angle)) * (radius_line) + center_x;   // calculate x pos for the line end
        line_y = -cos(radians(angle)) * radius_line + center_y;    // calculate y pos for the line end
        tft.drawLine(pixel_x, pixel_y, line_x, line_y, TFT_WHITE); // draw the line
      }
    }
    else
    {                                             // draw small tickmark == pixel tickmark
      tft.drawPixel(pixel_x, pixel_y, TFT_WHITE); // draw a single pixel
    }
  }

  // led
  if (desired_temp < (degreesF - 1))
  {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
  }
  else if (desired_temp > (degreesF + 1))
  {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
  }
  else
  {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
  }

/* display current temp on bottom */
  tft.setTextSize(3);
  sprintf(buffer, "%.1f%s", degreesF, " F");
  string_width = tft.textWidth(buffer); // calculate string width
  tft.setTextColor(TFT_WHITE);                           // set color to black
  tft.drawString(buffer, (240 - string_width) / 2, 100); // draw the value on top of the display
  tft.drawCircle(145, 105, 4, TFT_WHITE);                // degrees symbol
  tft.drawCircle(145, 105, 3, TFT_WHITE);                // degrees symbol

  delay(25);
}
