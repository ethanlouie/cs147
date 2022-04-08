#include <Arduino.h>
#include <stdio.h>
#include <Servo.h>


Servo myservo; // create servo object to control a servo

int photoresistor_pin = 2;
int servo_pin = -1;

uint16_t minimum = 4096;
uint16_t maximum = 0;

char buf[50];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(photoresistor_pin, INPUT);
  myservo.attach(17); // attaches the servo on port 17 to the servo object

}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() <= 10000) {
    int light = analogRead(photoresistor_pin); // 12 bit digital value (0-4095)
    float light_percent = light / 4095.0;

    if (light > maximum) {
      maximum = light;
    } else if (light < minimum) {
      minimum = light;
    }

    sprintf(buf, "millis: %d minimum: %d  maximum: %d  ", millis(), minimum, maximum);

    Serial.print(light_percent);
    Serial.print("  ");
    Serial.print(buf );
    Serial.println(light);

  }else {
    int light = analogRead(photoresistor_pin); // 12 bit digital value (0-4095)

    // 180, 900, 540 -> 540 * 360/(900-180)

    uint16_t angle = light *179 / (maximum - minimum);
    if (angle > 179) {
      angle = 0;
    }
    myservo.write(angle);
    delay(10);
  }

  //delay(10);


  // do servo things

}
