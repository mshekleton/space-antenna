#include "FastAccelStepper.h"

// As in StepperDemo for Motor 1 on AVR
//#define dirPinStepper    5
//#define enablePinStepper 6
//#define stepPinStepper   9  // OC1A in case of AVR

// As in StepperDemo for Motor 1 on ESP32
#define dirPinStepper 13
#define enablePinStepper 11
#define stepPinStepper 12
#define PULSES_PER_DEG 20

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

void setup() {
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper) {
    stepper->setDirectionPin(dirPinStepper);
    stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);

    // If auto enable/disable need delays, just add (one or both):
    // stepper->setDelayToEnable(50);
    // stepper->setDelayToDisable(1000);

    stepper->setSpeedInUs(1000);  // the parameter is us/step !!!
    stepper->setAcceleration(500);
    //stepper->move(-1000);


  Serial.begin(115200);
  Serial.println("Enter a number:");
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

      float azimuth_degrees = azimuth_str.toFloat();
      float altitude_degrees = altitude_str.toFloat();

      // Print the values to verify if they were correctly parsed (optional)
      Serial.print("Azimuth: ");
      Serial.print(azimuth_degrees);
      Serial.print(", Altitude: ");
      Serial.println(altitude_degrees);

      int pos = azimuth_degrees * PULSES_PER_DEG;
      stepper->moveTo(pos);

    } else {
      // If the input is not valid (no comma found), print an error message
      Serial.println("Invalid input. Please enter azimuth and altitude degrees separated by a comma (e.g., 100, 90):");
    }
  }
}
