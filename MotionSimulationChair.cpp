#include "FastAccelStepper.h"

// Define pins for the motors
#define STEPPER1_STEP_PIN 6
#define STEPPER1_DIR_PIN 3

#define STEPPER2_STEP_PIN 7
#define STEPPER2_DIR_PIN 4

#define STEPPER3_STEP_PIN 8
#define STEPPER3_DIR_PIN 5

// These values can be tuned
#define MAX_SPEED 10000
#define MAX_ACCELERATION 10000
#define MIN_SPEED 100
#define MIN_ACCELERATION 100

#define MAX_POS_PITCH_POSITION 83 // 30/(360/(5 * 200))
#define MAX_NEG_PITCH_POSITION -83

#define MAX_POS_ROLL_POSITION 83 // 30/(360/(5 * 200))
#define MAX_NEG_ROLL_POSITION -83

#define MAX_POS_YAW_POSITION 55 // 20/(360/(5 * 200))
#define MAX_NEG_YAW_POSITION -55

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

/**
 * Function to setup the Serial, steppers, and move the motors to the zero position if they are not already there 
*/
void setup() {
  Serial.begin(4800);
  while (!Serial); // Wait for the Serial to start
  engine.init(); // Start the engine

  // Initialize steppers (stepper1 and stepper2 are used for pitch and roll and stepper3 is used for yaw)
  stepper1 = engine.stepperConnectToPin(STEPPER1_STEP_PIN);
  if (stepper1) {
    stepper1 -> setDirectionPin(STEPPER1_DIR_PIN);
    stepper1 -> setSpeedInHz(1000); // steps/sec
    stepper1 -> setAcceleration(1000); // steps/sec^2
  }

  stepper2 = engine.stepperConnectToPin(STEPPER2_STEP_PIN);
  if (stepper2) {
    stepper2 -> setDirectionPin(STEPPER2_DIR_PIN);
    stepper2 -> setSpeedInHz(1000); // steps/sec
    stepper2 -> setAcceleration(1000); // steps/sec^2
  }

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

/**
 * Function to continuously loop through the enumeration states
*/
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

/**
 * Function to obtain the kind of experience that the user would like
*/
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

  // If the user provided 1, change the state to WAIT_FOR_AXIS_CHOICE
  // If the user provided 2 or 3, change the state to PROCESSING
  // If the user didn't provide 1, 2 or 3, do not change the state so that the program can ask the user again 
  if (choice.equals("1")) {
    currentState = WAIT_FOR_AXIS_CHOICE;

  } else if (choice.equals("2") || choice.equals("3")) {
    currentState = PROCESSING;

  } else {
    Serial.println("Invalid choice. Please enter 1 for single direction or 2 for full experience.");
    Serial.println();
  }
}

/**
 * Function to get the axis that user would like to move in
 */
void getAxis() { 
  Serial.println("Enter axis (Pitch/Roll/Yaw):");
  Serial.println();

  // Get the desired axis of movement
  while (!Serial.available()) {
  }
  String axisChoice = Serial.readStringUntil('\n');
  axisChoice.trim();

  // If the user provided a valid axis (pitch, roll, or yaw), set the desired axis
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

/**
 * Function to get the speed and acceleration that the user would like to move at
 */
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

  // If the provided speed and acceleration are within the parameters, set the motors speed and acceleration
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

/**
 * Function called to set the speed and acceleration for a specified stepper
 * @param stepper the stepper to set the speed and acceleration for
 * @param speed the speed to set the passed in stepper to
 * @param acceleration the acceleration to set the passed in stepper to
 */
void setMotorSpeedAndAcceleration(FastAccelStepper* stepper, float speed, float acceleration) {
  stepper->setSpeedInHz(speed); // steps/sec
  stepper->setAcceleration(acceleration); // steps/sec^2
}

/**
 * Function to get the position the user would like to move to
 */
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
    Serial.println("Enter a position in degrees (-20 to 20): ");
    Serial.println();
    while (!Serial.available()) {
    }    

    positionInput = Serial.readStringUntil('\n');
    positionInDegrees = positionInput.toFloat();

    if (positionInDegrees < -20) {
      positionInDegrees = -20;

    } else if (positionInDegrees > 20) {
      positionInDegrees = 20;
    }
  }

  // Utilize the calculatePosition function to convert the degrees to # of steps
  positionInSteps = calculatePosition(positionInDegrees);

  // Move to executing move of motor(s)
  currentState = PROCESSING;
}

/**
 * Function to calculate the number of steps needed to move to a certain number of degrees
 * 360.0 is the degrees per revolution
 * 5 is for the 1:5 ratio due to the gearbox
 * 200 is the numbers of steps in a full rotation due to normal stepping
 * @param positionInDegrees degrees to be converted into steps 
*/
int32_t calculatePosition(float positionInDegrees) {
  return floor((positionInDegrees/(360.0/(5 * 200))));
}

/**
 * Function to actually move the specified motor(s) to a specified position
 * @param position the number of steps to move to get to a certain position 
*/
void moveMotor(int32_t position) {
  // If the desired movement is pitch, move stepper1 and stepper2 in the same direction
  if (currentAxis.equalsIgnoreCase("Pitch")) {
    stepper1 -> moveTo(position, false);
    stepper2 -> moveTo(position, true);

    stepper1 -> moveTo(position * -1, false);
    stepper2 -> moveTo(position * -1, true);

    stepper1 -> moveTo(0, false);
    stepper2 -> moveTo(0, true);

  // If the desired movement is roll, move stepper1 and stepper2 in opposite directions
  } else if (currentAxis.equalsIgnoreCase("Roll")) {
    stepper1 -> moveTo(position, false);
    stepper2 -> moveTo(position * -1, true);

    stepper1 -> moveTo(position * -1, false);
    stepper2 -> moveTo(position, true);

    stepper1 -> moveTo(0, false);
    stepper2 -> moveTo(0, true);

  // If the desired movement is yaw, move stepper3
  } else if (currentAxis.equalsIgnoreCase("Yaw")) {
    stepper3 -> moveTo(position, true);

    stepper3 -> moveTo(position * -1, true);

    stepper3 -> moveTo(0, true);
  }
  // Go back to getting a new user desired movement
  currentState = WAIT_FOR_INPUT;
}

/**
 * Function that moves the chair in a multi-axial experience 
*/
void performFullExperienceMotion() {
  Serial.println("Moving all three motors!");
  Serial.println();
  
  // 30 degress up with two motors
  // Then -60 degrees down with two motors
  // Then 30 degrees up with two motors to go back to original location
  currentAxis = "pitch";
  moveMotor(MAX_POS_PITCH_POSITION);

  // 30 degrees up with one motor and -30 degrees down with the other
  // Then 60 degrees up with one motor and -60 degrees down with the other
  // Then 30 degrees up with one motor and -30 degrees down with the other to go back to original location.
  currentAxis = "roll";
  moveMotor(MAX_POS_ROLL_POSITION); 

  // 20 degrees yawing to the right
  // Then -40 degrees yawing to the left
  // Then 20 degrees yawing back to the original position
  currentAxis = "yaw";
  moveMotor(MAX_POS_YAW_POSITION);

  // Go back to getting a new user desired movement
  currentState = WAIT_FOR_INPUT;
}


/**
 * Function that mimics what it would be like to be on a roller coaster 
*/
void performRollerCoasterSimulation() {
  Serial.println("Starting Roller Coaster Simulation...");

  // Begin with a slow climb to the first peak
  Serial.println("Climbing to the first peak...");
  slowClimb();

  // Level out at the first peak
  Serial.println("Leveling out at the peak...");
  levelOut();

  // The first major drop
  Serial.println("Descending the first major drop...");
  fastFall();

  // Level out at the bottom
  Serial.println("Leveling out after the drop...");
  levelOut();

  // A series of smaller hills and dips
  Serial.println("Navigating smaller hills...");
  for (int i = 0; i < 4; i++) {
    smallHill();
    gentleDip();
  }

  // Level out after last dip
  Serial.println("Leveling out after hills and dips...");
  levelOut();

  // Introduce a sharp turn
  Serial.println("Executing a sharp turn...");
  sharpRightTurn();
  sharpLeftTurn();
  stepper3 -> moveTo(0, true);

  // Simulate a fast climb to another peak
  Serial.println("Fast climbing to another peak...");
  fastClimb();

  // Level out at the second peak
  Serial.println("Leveling out at the second peak...");
  levelOut();

  // Final major fall
  Serial.println("Descending the final major fall...");
  fastFall();

  // Level out at the bottom to end the ride
  Serial.println("Leveling out to end the ride...");
  levelOut();

  // End of the simulation
  Serial.println("Roller Coaster Ride Complete!");

  // Go back to getting a new user desired movement
  currentState = WAIT_FOR_INPUT;
}

/**
 * Function to perform a slow climb up a hill
*/
void slowClimb() {
  setMotorSpeedAndAcceleration(stepper1, 600, 300);
  setMotorSpeedAndAcceleration(stepper2, 600, 300);
  int climbingHeight;

  // Inch up in 10 increments to simulate a climb
  for (int i = 1; i <= 10; i++) {
    climbingHeight = MAX_NEG_PITCH_POSITION * (0.1 * i);
    stepper1 -> moveTo(climbingHeight, false);
    stepper2 -> moveTo(climbingHeight, true);
    delay(100);
  }
  // Stay at the max climb angle for 1 second
  delay(1000);
}

/**
 * Function to perform a fast climb
*/
void fastClimb() {
  setMotorSpeedAndAcceleration(stepper1, 2000, 1000); 
  setMotorSpeedAndAcceleration(stepper2, 2000, 1000);

  int climbingHeight = MAX_NEG_PITCH_POSITION;

  stepper1 -> moveTo(climbingHeight, false);
  stepper2 -> moveTo(climbingHeight, true);

  // Stay at max climb angle for 2 seconds
  delay(2000);
}

/**
 * Function to level out
*/
void levelOut() {
  int currentHeight = stepper1->getCurrentPosition();
  int levelingHeight;

  // Get the average speed to have a dynamic level out
  uint32_t speed1 = stepper1 -> getSpeedInMilliHz() * 1000;
  uint32_t speed2 = stepper2 -> getSpeedInMilliHz() * 1000;
  uint32_t averageSpeed = (speed1 + speed2) / 2;

  // The faster the speed, the smaller the leveling factor
  float levelingFactor = max(0.01, 1.0 - (averageSpeed / 1000.0));

  // The faster the speed, the longer the delay between each level out
  int dynamicDelay = min(500, max(100, int(averageSpeed / 10))); 

  // Level out in 10 increments based on the levelingFactor and the dynamicDelay
  for (int i = 9; i >= 0; i--) {
    levelingHeight = int(currentHeight * levelingFactor * i / 9);
    stepper1->moveTo(levelingHeight, false);
    stepper2->moveTo(levelingHeight, true);
    delay(dynamicDelay);
  }
}

/**
 * Function to perform a fast fall
*/
void fastFall() {
  setMotorSpeedAndAcceleration(stepper1, 3000, 1500);
  setMotorSpeedAndAcceleration(stepper2, 3000, 1500);

  int fallingHeight = MAX_POS_PITCH_POSITION;

  stepper1 -> moveTo(fallingHeight, false);
  stepper2 -> moveTo(fallingHeight, true);

  // Stay at the max fall angle for 2 seconds
  delay(2000);
}

/**
 * Function to perform a gentle dip
*/
void gentleDip() {
  // Dip is half the max angle possible
  int midHeight = MAX_POS_PITCH_POSITION / 2;

  stepper1 -> moveTo(midHeight, false);
  stepper2 -> moveTo(midHeight, true);

  // Stay on the dip for only half a second
  delay(500);
}

/**
 * Function to perform a small hill
*/
void smallHill() {
  // Hill is half the max angle possible
  int hillHeight = MAX_NEG_PITCH_POSITION / 2;

  stepper1 -> moveTo(hillHeight, false);
  stepper2 -> moveTo(hillHeight, true);

  // Stay on the hill for only half a second
  delay(500);
}

void sharpRightTurn() {
  setMotorSpeedAndAcceleration(stepper3, 2000, 1000);

  stepper3 -> moveTo(MAX_POS_YAW_POSITION, true);

  // Let the sharp turn last 300ms
  delay(300);
}

void sharpLeftTurn() {
  setMotorSpeedAndAcceleration(stepper3, 2000, 1000);

  stepper3 -> moveTo(MAX_NEG_YAW_POSITION, true);
  
  // Let the sharp turn last 300ms
  delay(300);
}

/**
 * Function to perform a twist
*/
void suddenTwist() {
  stepper1 -> setSpeedInHz(2000);

  // Roll in one direction and stay there for 200ms
  stepper1 -> moveTo(MAX_POS_ROLL_POSITION, false);
  stepper2 -> moveTo(MAX_NEG_ROLL_POSITION, true);
  delay(200);

  // Roll in the other direction and stay there for 200ms
  stepper1 -> moveTo(-MAX_POS_ROLL_POSITION, false);
  stepper2 -> moveTo(-MAX_NEG_ROLL_POSITION, true);
  delay(200);
}

/**
 * Function to perform a rumble of the chair
 * This function seems to be rather finicky when implemented with the other movement functions
 * @param numberOfRumbles how many rumbles should be performed (Ex: 40 rumbles is equal to 2 seconds of rumbles (50ms * 40))
*/
void rumble(int numberOfRumbles) {
  int pitchIntensity = 20;
  int rollIntensity = 30;
  int yawIntensity = 30; 
  
  int32_t pitchCurrentPosition = stepper1 -> getCurrentPosition();
  int32_t rollCurrentPosition = stepper2 -> getCurrentPosition();
  int32_t yawCurrentPosition = stepper3 -> getCurrentPosition();

  stepper1 -> setSpeedInHz(1000);
  stepper2 -> setSpeedInHz(1000);
  stepper3 -> setSpeedInHz(1000);

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

  // Make sure the motors are done moving before continuing
  while (stepper1 -> isRunning() || stepper2 -> isRunning() || stepper3 -> isRunning()) {
    delay(10);
  }

  // Move the steppers back to their original position before the rumble happened
  stepper1 -> moveTo(pitchCurrentPosition, true);
  stepper2 -> moveTo(rollCurrentPosition, true);
  stepper3 -> moveTo(yawCurrentPosition, true);
}
