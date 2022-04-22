#include <Arduino.h>
#include "Button2.h"

#define RED_PIN 22
#define YELLOW_PIN 17
#define GREEN_PIN 2
#define BUZZER_PIN 21

// Hard coded enumerator
#define RED_STATE 0
#define RED_YELLOW_STATE 1
#define YELLOW_STATE 2
#define GREEN_STATE 3
#define GREEN_TRANSITION_STATE 4

//time for each state
#define RED_MILLIS 10000
#define YELLOW_MILLIS 2000
#define GREEN_BUTTON_MILLIS 5000

/* Buttons */
#define BUTTONR_PIN 35
Button2 buttonR;

int tl_state;           // Traffic light state.
unsigned long tl_timer; // Traffic light timer.  (stores millis() call when transition states)
uint8_t buttonPressed;
uint8_t play;
uint8_t greenLightLoopCounter;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);

  // Configure LED pins as outputs. 
    pinMode(RED_PIN, OUTPUT); 
    pinMode(YELLOW_PIN, OUTPUT); 
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Initial state for states and timers.. 
    tl_state = GREEN_STATE;

    tl_timer = millis();

    buttonPressed = 0;
    greenLightLoopCounter = 0;
    play = 1;

    digitalWrite(GREEN_PIN, HIGH);

    buttonR.begin(BUTTONR_PIN);

}



void soundAndButtonCheck(uint8_t isGreen) {
  for (int i = 0; i < 5; i++) {
    buttonR.loop();
    if (buttonR.isPressed() && isGreen) {
      buttonPressed = 1;
      buttonR.read();
    }
    for (int j = 0; j < 25; j++) {
      if (play) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(1);
        digitalWrite(BUZZER_PIN, LOW);
        delay(1);
      } else {
        delay(2);
      }
    }
  }
}


void greenSoundAndButton() {
  greenLightLoopCounter++;
      if (play && greenLightLoopCounter == 6) {
        play = 0;
        greenLightLoopCounter = 0;
      } else if (!play && greenLightLoopCounter == 2) {
        play = 1;
        greenLightLoopCounter = 0;
      }
      soundAndButtonCheck(1);
}

void loop() {
  switch (tl_state) {
    case GREEN_STATE:
      greenSoundAndButton();
      if (buttonPressed) {
        tl_state = GREEN_TRANSITION_STATE;
        tl_timer = millis();
      }
      break;
    case GREEN_TRANSITION_STATE:
      greenSoundAndButton();
      if (millis() - tl_timer >= GREEN_BUTTON_MILLIS ) {
        tl_state = YELLOW_STATE;
        tl_timer = millis();
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(YELLOW_PIN, HIGH);
        buttonPressed = 0;
      }
      break;
    case YELLOW_STATE:
      delay(YELLOW_MILLIS);
      if (millis() - tl_timer >= YELLOW_MILLIS) {
        tl_state = RED_STATE;
        tl_timer = millis();
        digitalWrite(YELLOW_PIN, LOW);
        digitalWrite(RED_PIN, HIGH);
      }
      break;
    case RED_STATE:
      play = !play;
      soundAndButtonCheck(0);
      if (millis() - tl_timer >= RED_MILLIS) {
        tl_state = RED_YELLOW_STATE;
        tl_timer = millis();
        digitalWrite(YELLOW_PIN, HIGH);
        play = 0;
      }
      break;
    case RED_YELLOW_STATE:
      delay(YELLOW_MILLIS);
      if (millis() - tl_timer >= YELLOW_MILLIS) {
        tl_state = GREEN_STATE;
        tl_timer = millis();
        digitalWrite(RED_PIN, LOW);
        digitalWrite(YELLOW_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        play = 1;
        greenLightLoopCounter = 0;
      }
      break;
    default:
      break;
  }
}