#include "FastAccelStepper.h"

// Define pins for the motors
#define STEPPER1_STEP_PIN 6
#define STEPPER1_DIR_PIN 3

#define STEPPER2_STEP_PIN 7
#define STEPPER2_DIR_PIN 4

#define STEPPER3_STEP_PIN 8
#define STEPPER3_DIR_PIN 5

#define MAX_SPEED 80000
#define MAX_ACCELERATION 10000

#define MAX_POS_PITCH_POSITION 109 // 9.81/(360/(5 * 800))
#define MAX_NEG_PITCH_POSITION -109

#define MAX_POS_ROLL_POSITION 88 // 7.94/(360/(5 * 800))
#define MAX_NEG_ROLL_POSITION -88

#define MAX_POS_YAW_POSITION 500 // 45/(360/(5 * 800))
#define MAX_NEG_YAW_POSITION -500

// Create the engine and the steppers
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper1 = nullptr;
FastAccelStepper *stepper2 = nullptr;
FastAccelStepper *stepper3 = nullptr;

// Enumeration of states to structure the simulation process
enum State {
  WAIT_FOR_INPUT,
  WAIT_FOR_AXIS_CHOICE,
  WAIT_FOR_POSITION,
  WAIT_FOR_SPEED,
  PROCESSING
};

// Start at the first state
State currentState = WAIT_FOR_INPUT;

FastAccelStepper *selectedStepper1 = nullptr;
FastAccelStepper *selectedStepper2 = nullptr;
String currentAxis = "";
int32_t positionInSteps = 0;

void setup() {
  Serial.begin(4800);
  while (!Serial); // Wait for the Serial to start
  engine.init(); // Start the engine

  // Initialize pitch stepper
  stepper1 = engine.stepperConnectToPin(STEPPER1_STEP_PIN);
  if (stepper1) {
    stepper1 -> setDirectionPin(STEPPER1_DIR_PIN);
    stepper1 -> setSpeedInHz(6000); // steps/sec
    stepper1 -> setAcceleration(10000); // steps/sec^2
  }

  // Initialize roll stepper
  stepper2 = engine.stepperConnectToPin(STEPPER2_STEP_PIN);

  if (stepper2) {
    stepper2 -> setDirectionPin(STEPPER2_DIR_PIN);
    stepper2 -> setSpeedInHz(6000); // steps/sec
    stepper2 -> setAcceleration(10000); // steps/sec^2
  }

  // Initialize yaw stepper
  stepper3 = engine.stepperConnectToPin(STEPPER3_STEP_PIN);
  if (stepper3) {
    stepper3 -> setDirectionPin(STEPPER3_DIR_PIN);
    stepper3 -> setSpeedInHz(6000); // steps/sec
    stepper3 -> setAcceleration(10000); // steps/sec^2
  }

  // Move motors to the absolute 0 position
  stepper1->moveTo(0, false);
  stepper2->moveTo(0, false);
  stepper3->moveTo(0, false);
  
  // Begin the program
  Serial.println("Welcome to our motion simulation chair!");
  Serial.println();
}

// With the enum of different states, we can utilize switch cases to go through the states of simulation
void loop() {
  switch (currentState) {
    case WAIT_FOR_INPUT:
      getExperience();
      break;
    case WAIT_FOR_AXIS_CHOICE:
      break;
    case WAIT_FOR_SPEED:
      break;
    case WAIT_FOR_POSITION:
      break;
    case PROCESSING:
      break;
  }
}

// If the user provided 1, change the state to WAIT_FOR_AXIS_CHOICE
// If the user provided 2, change the state to PROCESSING
// If the user didn't provide 1 or 2, do not change the state so that the program can ask the user again
void getExperience() {
  Serial.println("What kind of user experience would you like?");
  Serial.println("1: Move in one direction");
  Serial.println("2: Full experience of motion");
  Serial.println();

  while (!Serial.available()) {
  }
  String choice = Serial.readStringUntil('\n');
  choice.trim();

  if (choice == "1") {
    currentState = WAIT_FOR_AXIS_CHOICE;
    getAxis();

  } else if (choice == "2") {
    currentState = PROCESSING;
    performFullExperienceMotion(stepper1, stepper2, stepper3);

  } else {
    Serial.println("Invalid choice. Please enter 1 for single direction or 2 for full experience.");
    Serial.println();
  }
}

// Function to get the axis that user would like to move in
void getAxis() { 
  Serial.println("Enter Axis (Pitch/Roll/Yaw):");
  Serial.println();

  while (!Serial.available()) {
  }
  String axisChoice = Serial.readStringUntil('\n');
  axisChoice.trim();

  if (axisChoice.equalsIgnoreCase("Pitch") || axisChoice.equalsIgnoreCase("Roll")) {
    selectedStepper1 = stepper1;
    selectedStepper2 = stepper2;

  } else if (axisChoice.equalsIgnoreCase("Yaw")) {
    selectedStepper1 = stepper3;
  } 

  else {
    Serial.println("Invalid axis. Please enter Pitch, Roll, or Yaw.");
    Serial.println();
    return;
  }

  currentAxis = axisChoice;
  currentState = WAIT_FOR_SPEED;
  getSpeedAndAcceleration();
}

// Function to get the speed and acceleration that the user would like to move at
void getSpeedAndAcceleration() {
  Serial.println("Enter the speed you want to move at (steps/second):");

  while (!Serial.available()) {
  }
  String speedInput = Serial.readStringUntil('\n');
  float speed = speedInput.toFloat();

  Serial.println("Enter the acceleration you want to move at (steps/second^2):");

  while (!Serial.available()) {
  }
  String accelerationInput = Serial.readStringUntil('\n');
  float acceleration = accelerationInput.toFloat();

  // If the provided speed and acceleration are within the parameters, set the motors speed and acceleration and call the getPosition() method
  if (speed > 0 && speed <= MAX_SPEED && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
    if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
      setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);

    } else if (currentAxis.equalsIgnoreCase("Yaw")) {
      setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
    }

    currentState = WAIT_FOR_POSITION;
    getPosition();

  // Otherwise, there is or are values that are not within the parameters
  } else {
    if (speed <= 0 && acceleration <= 0) {
      Serial.println("Speed and acceleration values are 0 or below. Setting to minimum speed and acceleration.");

      speed = 1;
      acceleration = 1;

      if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
        setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);

      } else if (currentAxis.equalsIgnoreCase("Yaw")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      }

      currentState = WAIT_FOR_POSITION;
      getPosition();

    } else if (speed > MAX_SPEED && acceleration > MAX_ACCELERATION) {
      Serial.println("Speed and acceleration values are above the specified limit. Setting to maximum speed and acceleration.");

      speed = MAX_SPEED;
      acceleration = MAX_ACCELERATION;

      if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
        setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);

      } else if (currentAxis.equalsIgnoreCase("Yaw")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      }

      currentState = WAIT_FOR_POSITION;
      getPosition();

    } else if (speed <= 0 && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
      Serial.println("Speed value is 0 or below. Setting to minimum speed.");

      speed = 0;

      if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
        setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);
      
      } else if (currentAxis.equalsIgnoreCase("Yaw")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      }

      currentState = WAIT_FOR_POSITION;
      getPosition();
    
    } else if (speed > MAX_SPEED && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
      Serial.println("Speed value is above the specified limit. Setting to maximum speed.");

      speed = MAX_SPEED;

      if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
        setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);

      } else if (currentAxis.equalsIgnoreCase("Yaw")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      }

      currentState = WAIT_FOR_POSITION;
      getPosition();
    
    } else if (speed > 0 && speed <= MAX_SPEED && acceleration <= 0) {
      Serial.println("Acceleration value is 0 or below. Setting to minimum acceleration.");

      acceleration = 1;

      if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
        setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);
      
      } else if (currentAxis.equalsIgnoreCase("Yaw")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      }

      currentState = WAIT_FOR_POSITION;
      getPosition();
    
    } else if (speed > 0 && speed <= MAX_SPEED && acceleration > MAX_ACCELERATION) {
      Serial.println("Acceleration value is above the specified limit. Setting to maximum acceleration.");

      acceleration = MAX_ACCELERATION;

      if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
        setMotorSpeedAndAcceleration(selectedStepper2, speed, acceleration);
      
      } else if (currentAxis.equalsIgnoreCase("Yaw")) {
        setMotorSpeedAndAcceleration(selectedStepper1, speed, acceleration);
      }

      currentState = WAIT_FOR_POSITION;
      getPosition();
    }
  }
}

// Function called to set the speed and acceleration for a specified stepper
void setMotorSpeedAndAcceleration(FastAccelStepper* stepper, uint32_t speed, uint32_t acceleration) {
  stepper->setSpeedInHz(speed); // steps/sec
  stepper->setAcceleration(acceleration); // steps/sec^2
}

// Function to get the position the user would like to move to
void getPosition() {
  String positionInput; // Variable to take in the angle input from user
  float positionInDegrees; // Variable to hold the positionInput to float conversion

  // Get wanted position in degrees if axis is pitch
  if (currentAxis.equalsIgnoreCase("Pitch")) {
    Serial.println("Enter a position in degrees (-9.81 to 9.81): ");

    while (!Serial.available()) {
    }

    positionInput = Serial.readStringUntil('\n');
    positionInDegrees = positionInput.toFloat();

    if (positionInDegrees < -9.81) {
      positionInDegrees = -9.81;
    
    } else if (positionInDegrees > 9.81) {
      positionInDegrees = 9.81;
    }

    positionInSteps = calculatePosition(positionInDegrees);
  } 

  // Get wanted position in degrees if axis is roll
  else if (currentAxis.equalsIgnoreCase("Roll")) {
    Serial.println("Enter a position in degrees (-7.94 to 7.94): ");

    while (!Serial.available()) {
    }    

    positionInput = Serial.readStringUntil('\n');
    positionInDegrees = positionInput.toFloat();

    if (positionInDegrees < -7.94) {
      positionInDegrees = -7.94;
    
    } else if (positionInDegrees > 7.94) {
      positionInDegrees = 7.94;
    }

    positionInSteps = calculatePosition(positionInDegrees);
  }

  // Get wanted position in degrees if axis is yaw
  else if (currentAxis.equalsIgnoreCase("Yaw")) {
    Serial.println("Enter a position in degrees (-45 to 45): ");

    while (!Serial.available()) {
    }    

    positionInput = Serial.readStringUntil('\n');
    positionInDegrees = positionInput.toFloat();

    if (positionInDegrees < -45) {
      positionInDegrees = -45;

    } else if (positionInDegrees > 45) {
      positionInDegrees = 45;
    }

    positionInSteps = calculatePosition(positionInDegrees);
  }

  // Print out the steps from the calculated degrees to steps, and call the moveMotor function to move the motor
  currentState = WAIT_FOR_INPUT;
  Serial.println("Position in steps: " + String(positionInSteps));
  Serial.println();

  if (currentAxis.equalsIgnoreCase("Pitch")) {
    moveMotor(selectedStepper1, positionInSteps);
    moveMotor(selectedStepper2, positionInSteps);

  } else if (currentAxis.equalsIgnoreCase("Roll")) {
    moveMotor(selectedStepper1, positionInSteps);
    moveMotor(selectedStepper2, positionInSteps * -1);

  } else if (currentAxis.equalsIgnoreCase("Yaw")) {
    moveMotor(selectedStepper1, positionInSteps);
  }
}

// Function to calculate the number of steps needed to move to a certain number of degrees
// positionInDegrees is the degrees to move
// 360.0 is the degrees per revolution
// 5 is for the 1:5 ratio due to the gearbox
// 800 is the numbers of steps in a full rotation due to quarter-stepping
int32_t calculatePosition(float positionInDegrees) {
  return floor((positionInDegrees/(360.0/(5 * 800))));
}

// Function to actually move the specified motor to a specified position
void moveMotor(FastAccelStepper* stepper, int32_t position) {
  stepper->moveTo(position, false);
  currentState = WAIT_FOR_INPUT;
}

// Function that moves the chair in a multi-axial experienc
void performFullExperienceMotion(FastAccelStepper* stepper1, FastAccelStepper* stepper2, FastAccelStepper* stepper3) {
  Serial.println("Moving all three motors!");
  Serial.println();
  
  // 9.81 degress up with two motors
  stepper1->moveTo(MAX_POS_PITCH_POSITION, false);
  stepper2->moveTo(MAX_POS_PITCH_POSITION, true);

  // Then -19.62 degrees down with two motors
  stepper1->moveTo(MAX_NEG_PITCH_POSITION, false);
  stepper2->moveTo(MAX_NEG_PITCH_POSITION, true);

  // Then 9.81 degrees up with two motors to go back to original location
  stepper1->moveTo(MAX_POS_PITCH_POSITION, false);
  stepper2->moveTo(MAX_POS_PITCH_POSITION, true);

  // Then 7.91 degrees up with one motor and -7.91 degrees down with the other
  stepper1->moveTo(MAX_POS_ROLL_POSITION, false);
  stepper2->moveTo(MAX_NEG_ROLL_POSITION, true);

  // Then 15.82 degrees up with one motor and -15.82 degrees down with the other
  stepper1->moveTo(MAX_NEG_ROLL_POSITION, false);
  stepper2->moveTo(MAX_POS_ROLL_POSITION, true);

  // Then 7.91 degrees up with one motor and -7.91 degrees down with the other to go back to original location.
  stepper1->moveTo(MAX_POS_ROLL_POSITION, false);
  stepper2->moveTo(MAX_NEG_ROLL_POSITION, true);

  // Then 45 degrees yawing to the right and -45 degrees yawing to the left
  stepper3->moveTo(MAX_POS_YAW_POSITION, true);
  stepper3->moveTo(MAX_NEG_YAW_POSITION, true);

  currentState = WAIT_FOR_INPUT;
}

// Work in progress
void performRollerCoasterSimulation(FastAccelStepper* stepper1, FastAccelStepper* stepper2, FastAccelStepper* stepper3) {
  Serial.println("Performing roller coaster simulation!");
  Serial.println();
}
