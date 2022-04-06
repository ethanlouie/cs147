#include <Arduino.h>

int photoresistor_pin = 2;
int servo_pin = -1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(photoresistor_pin, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  int light = analogRead(photoresistor_pin); // 12 bit digital value (0-4095)
  float light_percent = light / 4095.0;

  Serial.print(light_percent);
  Serial.print(" ");
  Serial.println(light);


  // do servo things
}
