#include "FastAccelStepper.h"

// As in StepperDemo for Motor 1 on ESP32
#define dirPinStepperAzimuth 14
#define dirPinStepperAzimuthLow 13
#define enablePinStepperAzimuth 12
#define enablePinStepperAzimuthLow 11
#define stepPinStepperAzimuth 10
#define stepPinStepperAzimuthLow 9
#define limitPinAzimuth 46
#define limitPinAzimuthLow 3

#define dirPinStepperAltitude 4
#define dirPinStepperAltitudeLow 5
#define enablePinStepperAltitude 6
#define enablePinStepperAltitudeLow 7
#define stepPinStepperAltitude 15
#define stepPinStepperAltitudeLow 16
#define limitPinAltitude 17
#define limitPinAltitudeLow 18

//const int sparePin1 = 25; // Replace with the GPIO pin number you want to use
//const int sparePin2 = 32; // Replace with another GPIO pin number you want to use

#define AZ_PULSES_PER_DEG 20
#define ALT_PULSES_PER_DEG 33.333 //200 steps/rev * 1:30 gearbox = 6000 steps/rev. 33.333 = (6000/360) * 2 (half stepping)
#define MAX_AZ_DEG 360
#define MIN_AZ_DEG -360
#define MAX_ALT_DEG 90
#define MIN_ALT_DEG 0

#define LED_BLUE HIGH
#define LED_GREEN LOW
#define LED_PIN 38 


FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperAzimuth = NULL;
FastAccelStepper *stepperAltitude = NULL;

// Flag to track which motor to control
bool isAzimuthMotorActive = true;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(dirPinStepperAzimuthLow, OUTPUT);
  pinMode(enablePinStepperAzimuthLow, OUTPUT);
  pinMode(stepPinStepperAzimuthLow, OUTPUT);
  pinMode(limitPinAzimuthLow, OUTPUT);

  digitalWrite(dirPinStepperAzimuthLow, LOW);
  digitalWrite(enablePinStepperAzimuthLow, HIGH);
  digitalWrite(stepPinStepperAzimuthLow, LOW);
  digitalWrite(limitPinAzimuthLow, LOW);
  
  pinMode(dirPinStepperAltitudeLow, OUTPUT);
  pinMode(enablePinStepperAltitudeLow, OUTPUT);
  pinMode(stepPinStepperAltitudeLow, OUTPUT);
  pinMode(limitPinAltitudeLow, OUTPUT);

  digitalWrite(dirPinStepperAltitudeLow, LOW);
  digitalWrite(enablePinStepperAltitudeLow, HIGH);
  digitalWrite(stepPinStepperAltitudeLow, LOW);
  digitalWrite(limitPinAltitudeLow, LOW);


  engine.init();

  stepperAzimuth = engine.stepperConnectToPin(stepPinStepperAzimuth);
  stepperAltitude = engine.stepperConnectToPin(stepPinStepperAltitude);

  if (stepperAzimuth && stepperAltitude) {
    // Set up the pins and configurations for Azimuth motor
    stepperAzimuth->setDirectionPin(dirPinStepperAzimuth);
    stepperAzimuth->setEnablePin(enablePinStepperAzimuth);
    stepperAzimuth->setAutoEnable(true);
    stepperAzimuth->setSpeedInUs(1000);  // the parameter is us/step !!!
    stepperAzimuth->setAcceleration(500);

    // Set up the pins and configurations for Altitude motor
    stepperAltitude->setDirectionPin(dirPinStepperAltitude);
    stepperAltitude->setEnablePin(enablePinStepperAltitude);
    stepperAltitude->setAutoEnable(true);
    stepperAltitude->setSpeedInUs(1000);  // the parameter is us/step !!!
    stepperAltitude->setAcceleration(500);

    Serial.println("Enter azimuth and altitude degrees separated by a comma (e.g., 100, 90):");
  }
}

void loop() {
  if (Serial.available() > 0) {
    // Read the input from the user
    String input = Serial.readStringUntil('\n'); // Read until newline character is received

    // Parse the input to extract azimuth and altitude degrees
    int commaIndex = input.indexOf(',');
    if (commaIndex != -1) {
      String azimuth_str = input.substring(0, commaIndex);
      String altitude_str = input.substring(commaIndex + 1);

      float azimuth_degrees = constrain(azimuth_str.toFloat(), MIN_AZ_DEG, MAX_AZ_DEG);
      float altitude_degrees = constrain(altitude_str.toFloat(), MIN_ALT_DEG, MAX_ALT_DEG);

      // Print the values to verify if they were correctly parsed (optional)
      //Serial.print("Azimuth: ");
      //Serial.print(azimuth_degrees);
      //Serial.print(", Altitude: ");
      //Serial.println(altitude_degrees);

      int azimuth_pos = azimuth_degrees * AZ_PULSES_PER_DEG;
      int altitude_pos = altitude_degrees * ALT_PULSES_PER_DEG;
      
      // Move both motors to their respective positions simultaneously
      stepperAzimuth->moveTo(azimuth_pos);
      stepperAltitude->moveTo(altitude_pos);

    } else {
      // If the input is not valid (no comma found), print an error message
      Serial.println("Invalid input. Please enter azimuth and altitude degrees separated by a comma (e.g., 100, 90):");
    }
  }

}
