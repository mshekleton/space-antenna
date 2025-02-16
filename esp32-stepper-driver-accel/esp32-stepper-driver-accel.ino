#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver.
#define DIR_PIN 13 // Direction
#define STEP_PIN 12 // Step
#define MOTOR_INTERFACE_TYPE 1

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed(10);
  stepper.setAcceleration(500);
  
  // Setup ESP32's internal Serial for debugging
  Serial.begin(115200);
}

void loop() {
  // Read the target position from the Serial (if available):
  //if (Serial.available() > 0) {
  //  String targetPosString = Serial.readStringUntil('\n');
  //  long targetPos = targetPosString.toInt();
    long targetPos = -10000;

    // Set the target position:
    stepper.moveTo(targetPos);
  //}

  // If the stepper has not yet reached its target, move it:
  if (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}
