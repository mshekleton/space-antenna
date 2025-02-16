#include "FastAccelStepper.h"

// As in StepperDemo for Motor 1 on AVR
//#define dirPinStepper    5
//#define enablePinStepper 6
//#define stepPinStepper   9  // OC1A in case of AVR

// As in StepperDemo for Motor 1 on ESP32
#define dirPinStepper 13
#define enablePinStepper 26
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
    //int number = Serial.parseInt();
    float deg = Serial.parseFloat();
    
    // Check if the input is valid (i.e., a number was successfully read)
    if (Serial.read() == '\n') {
      // Print the number back to the user
      Serial.print("You entered: ");
      //Serial.println(number);
      Serial.println(deg);
      //stepper->move(number);

      int pos = deg * PULSES_PER_DEG;
      stepper->moveTo(pos);
        
    } else {
      // If the input is not valid, clear the input buffer
      while (Serial.available() > 0) {
        Serial.read();
      }
      Serial.println("Invalid input. Please enter a number:");
    }
  }
  }
