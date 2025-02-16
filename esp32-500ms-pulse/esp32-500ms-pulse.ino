#include <Arduino.h>

const int ledPin = 12; // Pin 12 for the LED

void setup() {
  pinMode(ledPin, OUTPUT); // Set the LED pin as an output
}

void loop() {
  digitalWrite(ledPin, HIGH); // Turn on the LED
  delay(500); // Wait for 500 milliseconds
  digitalWrite(ledPin, LOW); // Turn off the LED
  delay(500); // Wait for another 500 milliseconds
}
