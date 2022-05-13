#include <Arduino.h>
#include <stdio.h>

#define RED_PIN 12
#define BLUE_PIN 13
#define GREEN_PIN 15
#define POTENTIOMETER_PIN 36

void toggleRedLED(void *parameter)
{
  for (;;)
  { // infinite loop
    // Turn the LED on
    digitalWrite(RED_PIN, HIGH);
    // Pause the task for 500ms
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // Turn the LED off
    digitalWrite(RED_PIN, LOW);
    // Pause the task again for 500ms
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void toggleBlueLED(void *parameter)
{
  for (;;)
  { // infinite loop
    // Turn the LED on
    digitalWrite(BLUE_PIN, HIGH);
    // Pause the task for 500ms
    vTaskDelay(250 / portTICK_PERIOD_MS);
    // Turn the LED off
    digitalWrite(BLUE_PIN, LOW);
    // Pause the task again for 500ms
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void toggleGreenLED(void *parameter)
{
  for (;;)
  { // infinite loop
    // Turn the LED on
    digitalWrite(GREEN_PIN, HIGH);
    // Pause the task for 500ms
    vTaskDelay(300 / portTICK_PERIOD_MS);
    // Turn the LED off
    digitalWrite(GREEN_PIN, LOW);
    // Pause the task again for 500ms
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(9200);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  xTaskCreate(
      toggleRedLED,    // Function that should be called
      "Toggle red LED", // Name of the task (for debugging)
      1000,         // Stack size (bytes)
      NULL,         // Parameter to pass
      1,            // Task priority
      NULL          // Task handle
  );
  xTaskCreate(
      toggleGreenLED,    // Function that should be called
      "Toggle green LED", // Name of the task (for debugging)
      1000,         // Stack size (bytes)
      NULL,         // Parameter to pass
      1,            // Task priority
      NULL          // Task handle
  );
  xTaskCreate(
      toggleBlueLED,    // Function that should be called
      "Toggle blue LED", // Name of the task (for debugging)
      1000,         // Stack size (bytes)
      NULL,         // Parameter to pass
      1,            // Task priority
      NULL          // Task handle
  );
}

void loop()
{
  uint64_t var = analogRead(POTENTIOMETER_PIN);
  Serial.println(var);
  delay(50);
}
