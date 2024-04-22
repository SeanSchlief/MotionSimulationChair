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
#define MIN_SPEED 1
#define MIN_ACCELERATION 1

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
  WAIT_FOR_SPEED_AND_ACCELERATION,
  WAIT_FOR_POSITION,
  PROCESSING
};

// Global variables //

// Start at the first state
State currentState = WAIT_FOR_INPUT;

// Pitch, roll, or yaw
String currentAxis = "";

// Desired movement
String choice = "";

// The # of steps to achieve a certain position
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
      getAxis();
      break;
    case WAIT_FOR_SPEED_AND_ACCELERATION:
      getSpeedAndAcceleration();
      break;
    case WAIT_FOR_POSITION:
      getPosition();
      break;
    case PROCESSING:
      if (choice == "1") {
        moveMotor(positionInSteps);

      } else if (choice == "2") {
        performFullExperienceMotion();

      } else if (choice == "3") {
        performRollerCoasterSimulation();
      }
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
  Serial.println("3: Roller coaster simulation");
  Serial.println();

  while (!Serial.available()) {
  }
  choice = Serial.readStringUntil('\n');
  choice.trim();

  if (choice.equals("1")) {
    currentState = WAIT_FOR_AXIS_CHOICE;

  } else if (choice.equals("2") || choice.equals("3")) {
    currentState = PROCESSING;

  } else {
    Serial.println("Invalid choice. Please enter 1 for single direction or 2 for full experience.");
    Serial.println();
  }
}

// Function to get the axis that user would like to move in
void getAxis() { 
  Serial.println("Enter axis (Pitch/Roll/Yaw):");
  Serial.println();

  // Get the desired axis of movement
  while (!Serial.available()) {
  }
  String axisChoice = Serial.readStringUntil('\n');
  axisChoice.trim();

  if (axisChoice.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll") || axisChoice.equalsIgnoreCase("Yaw")) {
    currentAxis = axisChoice;

  // Otherwise the axis is invalid and the user is asked again
  } else {
    Serial.println("Invalid axis. Please enter Pitch, Roll, or Yaw.");
    Serial.println();
    return;
  }

  // Move to getting the speed and acceleration
  currentAxis = axisChoice;
  currentState = WAIT_FOR_SPEED_AND_ACCELERATION;
}

// Function to get the speed and acceleration that the user would like to move at
void getSpeedAndAcceleration() {
  Serial.println("Enter the speed you want to move at (steps/second):");

  // Get the desired speed
  while (!Serial.available()) {
  }
  String speedInput = Serial.readStringUntil('\n');
  float speed = speedInput.toFloat();

  Serial.println("Enter the acceleration you want to move at (steps/second^2):");

  // Get the desired accleration
  while (!Serial.available()) {
  }
  String accelerationInput = Serial.readStringUntil('\n');
  float acceleration = accelerationInput.toFloat();

  // If the provided speed and acceleration are within the parameters, set the motors speed and acceleration and call the getPosition() method
  if (speed > 0 && speed <= MAX_SPEED && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
    if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
      setMotorSpeedAndAcceleration(stepper1, speed, acceleration);
      setMotorSpeedAndAcceleration(stepper2, speed, acceleration);

    } else if (currentAxis.equalsIgnoreCase("Yaw")) {
      setMotorSpeedAndAcceleration(stepper3, speed, acceleration);
    }

  // Otherwise, there is or are values that are not within the parameters
  } else {
    if (speed <= 0 && acceleration <= 0) {
      Serial.println("Speed and acceleration values are 0 or below. Setting to minimum speed and acceleration.");

      speed = MIN_SPEED;
      acceleration = MIN_ACCELERATION;

    } else if (speed > MAX_SPEED && acceleration > MAX_ACCELERATION) {
      Serial.println("Speed and acceleration values are above the specified limit. Setting to maximum speed and acceleration.");

      speed = MAX_SPEED;
      acceleration = MAX_ACCELERATION;

    } else if (speed <= 0 && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
      Serial.println("Speed value is 0 or below. Setting to minimum speed.");

      speed = MIN_SPEED;
    
    } else if (speed > MAX_SPEED && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
      Serial.println("Speed value is above the specified limit. Setting to maximum speed.");

      speed = MAX_SPEED;

    } else if (speed > 0 && speed <= MAX_SPEED && acceleration <= 0) {
      Serial.println("Acceleration value is 0 or below. Setting to minimum acceleration.");

      acceleration = MIN_ACCELERATION;
    
    } else if (speed > 0 && speed <= MAX_SPEED && acceleration > MAX_ACCELERATION) {
      Serial.println("Acceleration value is above the specified limit. Setting to maximum acceleration.");

      acceleration = MAX_ACCELERATION;
    }

    // With the fixed speeds and accelerations, we can set the speed and acceleration of the motor(s)
    if (currentAxis.equalsIgnoreCase("Pitch") || currentAxis.equalsIgnoreCase("Roll")) {
      setMotorSpeedAndAcceleration(stepper1, speed, acceleration);
      setMotorSpeedAndAcceleration(stepper2, speed, acceleration);

    } else if (currentAxis.equalsIgnoreCase("Yaw")) {
      setMotorSpeedAndAcceleration(stepper3, speed, acceleration);
    }
  }

  // Move to getting the desired position
  currentState = WAIT_FOR_POSITION;
}

// Function called to set the speed and acceleration for a specified stepper
void setMotorSpeedAndAcceleration(FastAccelStepper* stepper, float speed, float acceleration) {
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
  }

  positionInSteps = calculatePosition(positionInDegrees);

  Serial.println("Position in steps: " + String(positionInSteps));
  Serial.println();
  currentState = PROCESSING;
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
void moveMotor(int32_t position) {
  if (currentAxis.equalsIgnoreCase("Pitch")) {
    stepper1 -> moveTo(position, false);
    stepper2 -> moveTo(position, true);

    stepper1 -> moveTo(position * -1, false);
    stepper2 -> moveTo(position * -1, true);

    stepper1 -> moveTo(0, false);
    stepper2 -> moveTo(0, true);

  } else if (currentAxis.equalsIgnoreCase("Roll")) {
    stepper1 -> moveTo(position, false);
    stepper2 -> moveTo(position * -1, true);

    stepper1 -> moveTo(position * -1, false);
    stepper2 -> moveTo(position, true);

    stepper1 -> moveTo(0, false);
    stepper2 -> moveTo(0, true);

  } else if (currentAxis.equalsIgnoreCase("Yaw")) {
    stepper3 -> moveTo(position, true);

    stepper3 -> moveTo(position * -1, true);

    stepper3 -> moveTo(0, true);
  }
}

// Function that moves the chair in a multi-axial experience
void performFullExperienceMotion() {
  Serial.println("Moving all three motors!");
  Serial.println();
  
  // 9.81 degress up with two motors
  // Then -19.62 degrees down with two motors
  // Then 9.81 degrees up with two motors to go back to original location
  currentAxis = "pitch";
  moveMotor(MAX_POS_PITCH_POSITION);

  // 7.91 degrees up with one motor and -7.91 degrees down with the other
  // Then 15.82 degrees up with one motor and -15.82 degrees down with the other
  // Then 7.91 degrees up with one motor and -7.91 degrees down with the other to go back to original location.
  currentAxis = "roll";
  moveMotor(MAX_POS_ROLL_POSITION); 

  // 45 degrees yawing to the right
  // Then -90 degrees yawing to the left
  // Then 45 degrees yawing back to the original position
  currentAxis = "yaw";
  moveMotor(MAX_POS_YAW_POSITION);

  currentState = WAIT_FOR_INPUT;
}

// Function that mimics what it would be like to be on a roller coaster
void performRollerCoasterSimulation() {
  Serial.println("Performing advanced roller coaster simulation!");
  Serial.println();

  // Slow initial climb, a level out, and pause at the top
  slowClimb();
  levelOut();
  delay(2000);

  // First major hill 
  fastFall();
  levelOut();
  fastClimb();
  levelOut();

  gentleDip();
  for (int i = 0; i < 3; i++) {
    smallHill();
  }

  sharpTurn();
  suddenTwist();
  fastFall();
  levelOut();

  Serial.println("Roller coaster simulation complete!");
  currentState = WAIT_FOR_INPUT;
}

void slowClimb() {
  setMotorSpeedAndAcceleration(stepper1, 1000, 500);
  setMotorSpeedAndAcceleration(stepper2, 1000, 500);
  int climbingHeight;

  for (int i = 1; i <= 10; i++) {
    climbingHeight = MAX_POS_PITCH_POSITION * (0.1 * i);
    stepper1->moveTo(climbingHeight, false);
    rumble();
    stepper2->moveTo(climbingHeight, true);
    delay(250);
  }
}

void fastClimb () {

}

void levelOut() {
  int currentHeight = stepper1.getCurrentHeight();
  int levelingHeight;

  uint32_t speed1 = stepper1 -> getSpeedInHz();
  uint32_t speed2 = stepper2 -> getSpeedInHz();
  uint32_t averageSpeed = (speed1 + speed2) / 2;

  // Height lveling factor. The faster the speed, the smaller the leveling factor and vice versa
  float levelingFactor = max(0.01, 1.0 - (averageSpeed / 1000.0));

  // The faster the speed, the longer the delay between each level out and vice versa
  int delay = min(500, max(100, int(averageSpeed / 20))); // Delay ranging from 100ms to 500ms

  for (int i = 9; i >= 0; i--) {
    levelingHeight = currentHeight * levelingFactor * i;
    stepper1 -> moveTo(levelingHeight, false);
    stepper2 -> moveTo(levelingHeight, true);
    delay(delay)
  }
}

void fastFall() {
  setMotorSpeedAndAcceleration(stepper1, 3000, 1500);
  setMotorSpeedAndAcceleration(stepper2, 3000, 1500);

  int fallingHeight = -MAX_NEG_PITCH_POSITION;
  stepper1 -> moveTo(fallingHeight, false);
  stepper2 -> moveTo(fallingHeight, true);
  delay(1000);
}

void gentleDip() {

}

void sharpTurn() {

}

void suddenTwist() {

}

void rumble() {
  int pitchIntensity = 5;
  int rollInensity = 5;
  int yawIntensity = 10; 
  int duration = 1500; // Duration of rumble
  unsigned long startTime = millis();

  // Fast acceleration for quick response
  stepper1 -> setAcceleration(3000);
  stepper2 -> setAcceleration(3000);
  stepper3 -> setAcceleration(3000);

  // Randomly vary the intensity and direction of the steps
  while (millis() - startTime < duration) {
    stepper1 -> move(random(-pitchIntensity, pitchIntensity), false);
    stepper2 -> move(random(-rollIntensity, rollIntensity), false);
    stepper3 -> move(random(-yawIntensity, yawIntensity), false);
    delay(50); // Short delay between the shakes
  }
}
