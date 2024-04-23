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

#define MAX_POS_PITCH_POSITION 83 // 30/(360/(5 * 200))
#define MAX_NEG_PITCH_POSITION -83

#define MAX_POS_ROLL_POSITION 83 // 30/(360/(5 * 200))
#define MAX_NEG_ROLL_POSITION -83

#define MAX_POS_YAW_POSITION 125 // 45/(360/(5 * 200))
#define MAX_NEG_YAW_POSITION -125

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
    stepper1 -> setSpeedInHz(1000); // steps/sec
    stepper1 -> setAcceleration(1000); // steps/sec^2
  }

  // Initialize roll stepper
  stepper2 = engine.stepperConnectToPin(STEPPER2_STEP_PIN);

  if (stepper2) {
    stepper2 -> setDirectionPin(STEPPER2_DIR_PIN);
    stepper2 -> setSpeedInHz(1000); // steps/sec
    stepper2 -> setAcceleration(1000); // steps/sec^2
  }

  // Initialize yaw stepper
  stepper3 = engine.stepperConnectToPin(STEPPER3_STEP_PIN);
  if (stepper3) {
    stepper3 -> setDirectionPin(STEPPER3_DIR_PIN);
    stepper3 -> setSpeedInHz(1000); // steps/sec
    stepper3 -> setAcceleration(1000); // steps/sec^2
  }

  // Move motors to the absolute 0 position
  stepper1 -> moveTo(0, true);
  stepper2 -> moveTo(0, true);
  stepper3 -> moveTo(0, true);
  stepper1 -> setCurrentPosition(0);
  stepper2 -> setCurrentPosition(0);
  stepper3 -> setCurrentPosition(0);
  
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

  // Get the desired movement
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

  if (axisChoice.equalsIgnoreCase("Pitch") || axisChoice.equalsIgnoreCase("Roll") || axisChoice.equalsIgnoreCase("Yaw")) {
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
  Serial.println();

  // Get the desired speed
  while (!Serial.available()) {
  }
  String speedInput = Serial.readStringUntil('\n');
  float speed = speedInput.toFloat();

  Serial.println("Enter the acceleration you want to move at (steps/second^2):");
  Serial.println();

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
      Serial.println("Speed and acceleration values are 0 or below. Setting to " + String(MIN_SPEED) + " steps/s and " + String(MIN_ACCELERATION) + " steps/s^2.");
      Serial.println();

      speed = MIN_SPEED;
      acceleration = MIN_ACCELERATION;

    } else if (speed > MAX_SPEED && acceleration > MAX_ACCELERATION) {
      Serial.println("Speed and acceleration values are above the specified limit. Setting to " + String(MAX_SPEED) + " steps/s and " + String(MAX_ACCELERATION) + " steps/s^2.");
      Serial.println();

      speed = MAX_SPEED;
      acceleration = MAX_ACCELERATION;

    } else if (speed <= 0 && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
      Serial.println("Speed value is 0 or below. Setting to " + String(MIN_SPEED) + " steps/s.");
      Serial.println();

      speed = MIN_SPEED;
    
    } else if (speed > MAX_SPEED && acceleration > 0 && acceleration <= MAX_ACCELERATION) {
      Serial.println("Speed value is above the specified limit. Setting to " + String(MAX_SPEED) + " steps/s.");
      Serial.println();

      speed = MAX_SPEED;

    } else if (speed > 0 && speed <= MAX_SPEED && acceleration <= 0) {
      Serial.println("Acceleration value is 0 or below. Setting to " + String(MIN_ACCELERATION) + " steps/s^2.");
      Serial.println();

      acceleration = MIN_ACCELERATION;
    
    } else if (speed > 0 && speed <= MAX_SPEED && acceleration > MAX_ACCELERATION) {
      Serial.println("Acceleration value is above the specified limit. Setting to " + String(MAX_ACCELERATION) + " steps/s^2.");
      Serial.println();

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
    Serial.println("Enter a position in degrees (-30 to 30): ");
    Serial.println();

    while (!Serial.available()) {
    }

    positionInput = Serial.readStringUntil('\n');
    positionInDegrees = positionInput.toFloat();

    if (positionInDegrees < -30) {
      positionInDegrees = -30;
    
    } else if (positionInDegrees > 30) {
      positionInDegrees = 30;
    }
  } 

  // Get wanted position in degrees if axis is roll
  else if (currentAxis.equalsIgnoreCase("Roll")) {
    Serial.println("Enter a position in degrees (-30 to 30): ");
    Serial.println();

    while (!Serial.available()) {
    }    

    positionInput = Serial.readStringUntil('\n');
    positionInDegrees = positionInput.toFloat();

    if (positionInDegrees < -30) {
      positionInDegrees = -30;
    
    } else if (positionInDegrees > 30) {
      positionInDegrees = 30;
    }
  }

  // Get wanted position in degrees if axis is yaw
  else if (currentAxis.equalsIgnoreCase("Yaw")) {
    Serial.println("Enter a position in degrees (-45 to 45): ");
    Serial.println();

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
  return floor((positionInDegrees/(360.0/(5 * 200))));
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
  currentState = WAIT_FOR_INPUT;
}

// Function that moves the chair in a multi-axial experience
void performFullExperienceMotion() {
  Serial.println("Moving all three motors!");
  Serial.println();
  
  // 30 degress up with two motors
  // Then -60 degrees down with two motors
  // Then 30 degrees up with two motors to go back to original location
  currentAxis = "pitch";
  moveMotor(MAX_POS_PITCH_POSITION);

  // 30 degrees up with one motor and -30 degrees down with the other
  // Then 30 degrees up with one motor and -30 degrees down with the other
  // Then 30 degrees up with one motor and -30 degrees down with the other to go back to original location.
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
  Serial.println("Starting Roller Coaster Simulation...");

  // Begin with a slow climb to the first peak
  Serial.println("Climbing to the first peak...");
  slowClimb();
  rumble(60); // 3 seconds of rumbles as it climbs

  // Level out at the first peak
  Serial.println("Leveling out at the peak...");
  levelOut();
  rumble(20); // Little 1 second rumble at the peak

  // The first major drop
  Serial.println("Descending the first major drop...");
  fastFall();
  rumble(40); // 2 seconds of rumbles as it falls 

  // Level out at the bottom
  Serial.println("Leveling out after the drop...");
  levelOut();

  // A series of smaller hills and dips
  Serial.println("Navigating smaller hills...");
  for (int i = 0; i < 4; i++) {
      smallHill();
      gentleDip();
      if (i % 2 == 0) {
        rumble(10); // Half second rumbles on every second hill
      }
  }

  // Level out after last dip
  Serial.println("Leveling out after hills and dips...");
  levelOut();

  // Introduce a sharp turn
  Serial.println("Executing a sharp turn...");
  sharpRightTurn();
  rumble(20); // Rumble after turning right
  sharpLeftTurn();
  rumble(20); // Rumble after turning left
  stepper3 -> moveTo(0, true);

  // Add a sudden twist for extra thrill
  Serial.println("Adding a sudden twist...");
  suddenTwist();
  rumble(20); // Rumble after twists
  levelOut();

  // Simulate a fast climb to another peak
  Serial.println("Fast climbing to another peak...");
  fastClimb();
  rumble(40); // 2 second rumble while climbing

  // Level out at the second peak
  Serial.println("Leveling out at the second peak...");
  levelOut();

  // Final major fall
  Serial.println("Descending the final major fall...");
  fastFall();
  rumble(60); // 3 second rumble while going down the last hill

  // Level out at the bottom to end the ride
  Serial.println("Leveling out to end the ride...");
  levelOut();

  // End of the simulation
  Serial.println("Roller Coaster Ride Complete!");

  currentState = WAIT_FOR_INPUT;
}

void slowClimb() {
  setMotorSpeedAndAcceleration(stepper1, 600, 300);
  setMotorSpeedAndAcceleration(stepper2, 600, 300);
  int climbingHeight;

  for (int i = 1; i <= 10; i++) {
    climbingHeight = MAX_NEG_PITCH_POSITION * (0.1 * i);
    stepper1 -> moveTo(climbingHeight, false);
    stepper2 -> moveTo(climbingHeight, true);
    delay(100);
  }
  delay(1000);
}

void fastClimb() {
    setMotorSpeedAndAcceleration(stepper1, 2000, 1000); 
    setMotorSpeedAndAcceleration(stepper2, 2000, 1000);

    int climbingHeight = MAX_NEG_PITCH_POSITION;

    stepper1 -> moveTo(climbingHeight, false);
    stepper2 -> moveTo(climbingHeight, true);

    delay(2000);
}

void levelOut() {
    int currentHeight = stepper1->getCurrentPosition();
    int levelingHeight;

    uint32_t speed1 = stepper1 -> getSpeedInMilliHz() * 1000;
    uint32_t speed2 = stepper2 -> getSpeedInMilliHz() * 1000;
    uint32_t averageSpeed = (speed1 + speed2) / 2;

    // The faster the speed, the smaller the leveling factor
    float levelingFactor = max(0.01, 1.0 - (averageSpeed / 1000.0));

    // The faster the speed, the longer the delay between each level out
    int dynamicDelay = min(500, max(100, int(averageSpeed / 10))); 

    for (int i = 9; i >= 0; i--) {
        levelingHeight = int(currentHeight * levelingFactor * i / 9);
        stepper1->moveTo(levelingHeight, false);
        stepper2->moveTo(levelingHeight, true);
        delay(dynamicDelay);
    }
}

void fastFall() {
  setMotorSpeedAndAcceleration(stepper1, 3000, 1500);
  setMotorSpeedAndAcceleration(stepper2, 3000, 1500);

  int fallingHeight = MAX_POS_PITCH_POSITION;

  stepper1 -> moveTo(fallingHeight, false);
  stepper2 -> moveTo(fallingHeight, true);
  delay(2000);
}

void gentleDip() {
    int midHeight = MAX_POS_PITCH_POSITION / 2;

    stepper1 -> moveTo(midHeight, false);
    stepper2 -> moveTo(midHeight, true);
    delay(500);
}

void smallHill() {
    int hillHeight = MAX_NEG_PITCH_POSITION / 2;

    stepper1 -> moveTo(hillHeight, false);
    stepper2 -> moveTo(hillHeight, true);
    delay(500);
}

void sharpRightTurn() {
    setMotorSpeedAndAcceleration(stepper3, 2000, 1000);

    stepper3 -> moveTo(MAX_POS_YAW_POSITION, true);
    delay(300);
}

void sharpLeftTurn() {
    setMotorSpeedAndAcceleration(stepper3, 2000, 1000);

    stepper3 -> moveTo(MAX_NEG_YAW_POSITION, true);
    delay(300);
}

void suddenTwist() {
  stepper1 -> setSpeedInHz(2000);

  stepper1 -> moveTo(MAX_POS_ROLL_POSITION, false);
  stepper2 -> moveTo(MAX_NEG_ROLL_POSITION, true);
  delay(200);

  stepper1 -> moveTo(-MAX_POS_ROLL_POSITION, false);
  stepper2 -> moveTo(-MAX_NEG_ROLL_POSITION, true);
  delay(200);
}

void rumble(int numberOfRumbles) {
  int pitchIntensity = 20;
  int rollIntensity = 30;
  int yawIntensity = 30; 
  
  int32_t pitchCurrentPosition = stepper1 -> getCurrentPosition();
  int32_t rollCurrentPosition = stepper2 -> getCurrentPosition();
  int32_t yawCurrentPosition = stepper3 -> getCurrentPosition();

  // Fast acceleration for quick response
  stepper1 -> setSpeedInHz(1000);
  stepper2 -> setSpeedInHz(1000);
  stepper3 -> setSpeedInHz(1000);

  // Fast acceleration for quick response
  stepper1 -> setAcceleration(1000);
  stepper2 -> setAcceleration(1000);
  stepper3 -> setAcceleration(1000);

  // Randomly vary the intensity and direction of the steps
  for (int i = 0; i < numberOfRumbles; i++) {
    stepper1 -> move(random(-pitchIntensity, pitchIntensity), false);
    stepper2 -> move(random(-rollIntensity, rollIntensity), false);
    stepper3 -> move(random(-yawIntensity, yawIntensity), true);
    delay(50); // Short delay between the shakes
  }
  
  while (stepper1 -> isRunning() || stepper2 -> isRunning() || stepper3 -> isRunning()) {
    delay(10);
  }

  stepper1 -> moveTo(pitchCurrentPosition, true);
  stepper2 -> moveTo(rollCurrentPosition, true);
  stepper3 -> moveTo(yawCurrentPosition, true);
}
