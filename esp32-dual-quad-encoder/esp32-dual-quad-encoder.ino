volatile long encoderPos1 = 0, encoderPos2 = 0;
volatile bool encoderALast1 = LOW, encoderALast2 = LOW;
const int encoderPinA1 = 13, encoderPinA2 = 27;
const int encoderPinB1 = 12, encoderPinB2 = 14;
const float position_factor1 = 100;
const int int_position_factor1 = (int)position_factor1;
const float position_factor2 = 50;
const int int_position_factor2 = (int)position_factor2;
const long encoder1PosMin = -360;
const long encoder1PosMax = 360;
const long encoder2PosMin = -90;
const long encoder2PosMax = 90;

void IRAM_ATTR encoderInterrupt1() {
  int encoderA = digitalRead(encoderPinA1);
  int encoderB = digitalRead(encoderPinB1);
  if (encoderA != encoderALast1) {
    if (encoderA != encoderB) {
      encoderPos1++;
    } else {
      encoderPos1--;
    }
  }
  encoderALast1 = encoderA;
  encoderPos1 = constrain(encoderPos1, encoder1PosMin * int_position_factor1, encoder1PosMax * int_position_factor1);
}

void IRAM_ATTR encoderInterrupt2() {
  int encoderA = digitalRead(encoderPinA2);
  int encoderB = digitalRead(encoderPinB2);
  if (encoderA != encoderALast2) {
    if (encoderA != encoderB) {
      encoderPos2++;
    } else {
      encoderPos2--;
    }
  }
  encoderALast2 = encoderA;
  encoderPos2 = constrain(encoderPos2, encoder2PosMin * int_position_factor2, encoder2PosMax * int_position_factor2);
}

void setup() {
  Serial.begin(115200);
  pinMode(encoderPinA1, INPUT_PULLUP);
  pinMode(encoderPinB1, INPUT_PULLUP);
  pinMode(encoderPinA2, INPUT_PULLUP);
  pinMode(encoderPinB2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA1), encoderInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB1), encoderInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinA2), encoderInterrupt2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB2), encoderInterrupt2, CHANGE);
}

void loop() {
  Serial.print(encoderPos1/position_factor1, 2);
  Serial.print(", ");
  Serial.println(encoderPos2/position_factor2, 2);
  delay(100);
}
