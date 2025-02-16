volatile long encoderPos = 0;  // a counter for the dial
volatile bool encoderALast = LOW; // keep track of last A position
const int encoderPinA = 13;
const int encoderPinB = 12;
const float position_factor = 100;
const int int_position_factor = (int)position_factor;
const long encoderPosMin = -360;
const long encoderPosMax = 360;

void IRAM_ATTR encoderInterrupt() {
  int encoderA = digitalRead(encoderPinA); // Reads the current position of A
  int encoderB = digitalRead(encoderPinB); // Reads the current position of B

  if (encoderA != encoderALast) { // If A has changed
    if (encoderA != encoderB) { // and A & B are not equal
      encoderPos++; // it is a clockwise movement
    } else {
      encoderPos--; // it is a counter-clockwise movement
    }
  }
  encoderALast = encoderA; // Store A for next time
  encoderPos = constrain(encoderPos, encoderPosMin * int_position_factor, encoderPosMax * int_position_factor);
  
}

void setup() {
  Serial.begin(115200); // start serial for output
  pinMode(encoderPinA, INPUT_PULLUP); // set encoderPinA as input
  pinMode(encoderPinB, INPUT_PULLUP); // set encoderPinB as input

  // Call encoderInterrupt() when the inputs change (RISING, FALLING or CHANGE)
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), encoderInterrupt, CHANGE);
}

void loop() {
  // Print the encoder position to the Serial Monitor
  Serial.println(encoderPos/position_factor);
  delay(100); // for readability in the Serial Monitor
}
