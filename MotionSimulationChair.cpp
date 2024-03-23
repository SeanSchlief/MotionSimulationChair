#include <AccelStepper.h>

#define motorInterfaceType AccelStepper::FULL2WIRE

// Function definitions for C++ compiler
void promptForExperience();
void processExperienceChoice(const String& choice);
void processAxisChoice(const String& axisChoice);
void processSpeedInput(float speed);
void processPositionInput(long position);
void setMotorSpeed(AccelStepper& stepper, float speed);
void setMotorPosition(AccelStepper& stepper, long position);
void moveMotor(AccelStepper& stepper);
void performFullExperienceMotion();
void updateFullExperienceMotion();
void checkMotionCompletion();

// Motor pin definitions for each motor
const int pitchMotorStepPin = 2;
const int pitchMotorDirPin = 4;

const int rollMotorStepPin = 10;
const int rollMotorDirPin = 4;

const int yawMotorStepPin = 11;
const int yawMotorDirPin = 2;

// Values used for the full motion experience
int fullExperienceStep = 0;
long maxPosPitchPosition = 4000;
long maxPosRollPosition = 4000;
long maxPosYawPosition = 4000;
long maxNegPitchPosition = -4000;
long maxNegRollPosition = -4000;
long maxNegYawPosition = -4000;

// Instantiate AccelStepper objects
AccelStepper pitchStepper(motorInterfaceType, pitchMotorStepPin, pitchMotorDirPin);
AccelStepper rollStepper(motorInterfaceType, rollMotorStepPin, rollMotorDirPin);
AccelStepper yawStepper(motorInterfaceType, yawMotorStepPin, yawMotorDirPin);

// States to help navigate through simulating the motion
/* Steps:
   1. WAIT_FOR_INPUT: Does the user want to move in one axis or do they want a full experience?
   2. WAIT_FOR_AXIS_CHOICE: If they want to move in one axis, which axis do they want to move in?
   3. WAIT_FOR_POSITION: If they want to move in one axis, position would they like to move to?
   4. WAIT_FOR_SPEED: If they want to move in one axis, what speed would they like to move at?
   5. PROCESSING: Busy/In-Progres state to execute tasks
*/
enum State {
  WAIT_FOR_INPUT,
  WAIT_FOR_AXIS_CHOICE,
  WAIT_FOR_POSITION,
  WAIT_FOR_SPEED,
  PROCESSING,
};

// Start at the first state
State currentState = WAIT_FOR_INPUT;
AccelStepper* selectedStepper = nullptr; // Pointer to the currently selected stepper
String currentAxis = "";

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for Serial port to connect

  // Setup the steppers with initial speed and acceleration
  // Numbers will have to be adjusted
  pitchStepper.setMaxSpeed(1000);
  pitchStepper.setAcceleration(200);
  rollStepper.setMaxSpeed(1000);
  rollStepper.setAcceleration(200);
  yawStepper.setMaxSpeed(1000);
  yawStepper.setAcceleration(200);
  
  // True beginning of the program
  Serial.println("Welcome to our motion simulation chair!");
  promptForExperience();
}

// With the enum of different states, we can utilize switch cases to go through the states of simulation
void loop() {
  switch (currentState) {
    case WAIT_FOR_INPUT:
      if (Serial.available()) {
        String choice = Serial.readStringUntil('\n');
        processExperienceChoice(choice);
      }
      break;
    case WAIT_FOR_AXIS_CHOICE:
      if (Serial.available()) {
        String axisChoice = Serial.readStringUntil('\n');
        axisChoice.trim();
        processAxisChoice(axisChoice);
      }
      break;
    case WAIT_FOR_SPEED:
      if (Serial.available()) {
        String speedInput = Serial.readStringUntil('\n');
        float speed = speedInput.toFloat();
        processSpeedInput(speed);
      }
      break;
    case WAIT_FOR_POSITION:
      if (Serial.available()) {
        String posInput = Serial.readStringUntil('\n');
        long position = posInput.toInt();
        processPositionInput(position);
      }
      break;
    case PROCESSING:
      if (fullExperienceStep > 0) {
        updateFullExperienceMotion();
      }
      else {
        moveMotor(*selectedStepper);
      }
      checkMotionCompletion();
      break;
  }
}

// Function for getting the experience that the user wants
void promptForExperience() {
  Serial.println("What kind of user experience would you like?");
  Serial.println("1: Move in one direction");
  Serial.println("2: Full experience of motion");
}

// choice is the value provided from the promptForExperience function
// If the user provided 1, change the state to WAIT_FOR_AXIS_CHOICE
// If the user provided 2, change the state to PROCESSING
// If the user didn't provide 1 or 2, do not change the state so that the program can ask the user again
void processExperienceChoice(const String& choice) {
  if (choice == "1") {
    currentState = WAIT_FOR_AXIS_CHOICE;
    Serial.println("Enter Axis (Pitch/Roll/Yaw):");
  } else if (choice == "2") {
    currentState = PROCESSING;
    performFullExperienceMotion();
  } else {
    Serial.println("Invalid choice. Please enter 1 for single direction or 2 for full experience.");
  }
}

// axisChoice is the value provided from the processExperienceChoice function in the choice == "1" if statement
// If a valid axisChoice is provided, update the selectedStepper for the desired axis of motion, the axis, and the currentState to get the speed
void processAxisChoice(const String& axisChoice) {
  if (axisChoice.equalsIgnoreCase("Pitch")) {
    selectedStepper = &pitchStepper;
  } else if (axisChoice.equalsIgnoreCase("Roll")) {
    selectedStepper = &rollStepper;
  } else if (axisChoice.equalsIgnoreCase("Yaw")) {
    selectedStepper = &yawStepper;
  } else {
    Serial.println("Invalid axis. Please enter Pitch, Roll, or Yaw.");
    return;
  }
  currentAxis = axisChoice;
  currentState = WAIT_FOR_SPEED;
  Serial.println("Enter the speed you want to move at (steps/second):");
}

// speed is the value provided from the processAxisChoice function at the end
void processSpeedInput(float speed) {
  // If the stepper is not null, the speed is greater than 0, and the speed does not exceed the maxSpeed of the stepper motor, use the setMotorSpeed function
  // to update the speed of the specific motor. Also, ask the user for the position they want to move to and update the currentState to the WAIT_FOR_POSITION state
  // Otherwise, the user did not provide a speed that is valid and we have to re-ask for a valid speed
  if (selectedStepper != NULL && speed > 0 && speed <= selectedStepper->maxSpeed()) {
    setMotorSpeed(*selectedStepper, speed);
    Serial.println("Speed set. Enter the position (0 to maximum):");
    currentState = WAIT_FOR_POSITION;
  } else {
    Serial.println("Invalid speed. Please enter a positive number up to the max speed.");
    currentState = WAIT_FOR_SPEED;
  }
}

// position is the value provided from the processSpeedInput function at the end 
void processPositionInput(long position) {
  // If the stepper is not null, the position is greater than 0, and the position does not exceed the position of 4000 for the motor, use the setMotorPosition function
  // to move the motor to the chosen position. Also, update the currentState to PROCESSING, as the motor is now moving and we do not want any user input.
  if (selectedStepper != NULL && position >= 0 && position <= 4000) { // Assuming 4000 is the max
    currentState = PROCESSING;
    setMotorPosition(*selectedStepper, position);
  } else {
    Serial.println("Position out of range. Please enter a value between 0 and 4000.");
    currentState = WAIT_FOR_POSITION;
  }
}

// Function to update the speed of the selected motor
void setMotorSpeed(AccelStepper& stepper, float speed) {
  stepper.setMaxSpeed(speed);
  Serial.print("Speed set to: ");
  Serial.println(speed);
}

// Function to set the position for the motor to move to
void setMotorPosition(AccelStepper& stepper, long position) {
  currentState = PROCESSING;  
  stepper.moveTo(position);
  Serial.print("Moving to position: ");
  Serial.println(position);
}

// Function to move the motor to the desired position
void moveMotor(AccelStepper& stepper) {
    while (stepper.distanceToGo() != 0) {
        stepper.run();
    }
}

// Function to update the fullExperienceStep value to 1, so that the full experience can be started
void performFullExperienceMotion() {
  fullExperienceStep = 1;
}

// Fucntion that utilizes a switch case to go through movements in all three degrees of freedom
void updateFullExperienceMotion() {
  // Called from the loop() function when in the PROCESSING state
  switch (fullExperienceStep) {
    case 1:
      // Pitch up movement
      setMotorPosition(pitchStepper, maxPosPitchPosition);
      moveMotor(pitchStepper);
      fullExperienceStep++;
      break;
    case 2:
      // Pitch down movement
      setMotorPosition(pitchStepper, maxNegPitchPosition);
      moveMotor(pitchStepper);
      fullExperienceStep++;
      break;
    case 3:
      // Roll right movement
      setMotorPosition(rollStepper, maxPosRollPosition);
      moveMotor(rollStepper);
      fullExperienceStep++;
      break;
    case 4:
      // Roll left movement
      setMotorPosition(rollStepper, maxNegRollPosition);
      moveMotor(rollStepper);
      fullExperienceStep++;
      break;
    case 5:
      // Yaw right movement
      setMotorPosition(yawStepper, maxNegYawPosition);
      moveMotor(yawStepper);
      fullExperienceStep++;
      break;
    case 6:
      // Yaw left movement
      setMotorPosition(yawStepper, maxNegYawPosition);
      moveMotor(yawStepper);
      fullExperienceStep++;
      break;
  }
}

// Function to make sure the motors are done moving
void checkMotionCompletion() {
  // Check if all steppers have reached their target positions
  if (!pitchStepper.isRunning() && !rollStepper.isRunning() && !yawStepper.isRunning()) {
    Serial.println("Motion completed. Ready for new input.");
    currentState = WAIT_FOR_INPUT; // Go back to waiting for input
    promptForExperience(); // Prompt the user again for the experience they want
  } 
}
