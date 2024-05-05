# Motion Simulation Chair

Welcome to our **Motion Simulation Chair** project! This repository contains the code and documentation for a motion simulation chair designed as a capstone project for the University of Georgia.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Features](#features)
3. [Hardware Requirements](#hardware-requirements)
4. [Software Requirements](#software-requirements)
5. [Setup](#setup)
6. [Usage](#usage)
7. [Contributors](#contributors)

## Project Overview

The Motion Simulation Chair is a fully-functional, open-source, and modular product providing accurate motion simulation experiences for various applications like driving, flying, roller coasters, video/arcade games, and more. The chair provides pitch, roll, and yaw movements using stepper motors controlled by an Arduino-based system.

## Features

1. **Three Degrees of Freedom**: The chair provides pitch, roll, and yaw movements.
2. **Customizable Motion Profiles**: Users can select different motion experiences through a serial interface.
3. **Preset Experiences**: The software includes a full motion experience and a roller coaster simulation.
4. **Safety Features**: The software includes safety checks for speed, acceleration, and position to ensure safe operation.

## Hardware Requirements

1. **Stepper Motors**: Three NEMA 34 9Nm stepper motors.
2. **Motor Drivers**: Three CL86T high current stepper drivers.
3. **Power Supply**: Three 350W 48V DC power supplies.
4. **Microcontroller**: Arduino MEGA 2560.
5. **Other Components**: Casters, universal joint, structural frame, safety harness.

## Software Requirements

1. **Arduino IDE**: The code is written in C++ using the Arduino framework.
2. **Libraries**: 
   - [FastAccelStepper](https://github.com/gin66/FastAccelStepper)

## Setup

1. **Hardware Setup**:
   - Connect the motors, drivers, and power supplies according to the wiring diagram (provided in the documentation).
   - Mount the chair securely to the frame.

2. **Software Setup**:
   - Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
   - Clone this repository or download the source code.
   - Open the code in the Arduino IDE.
   - Install the `FastAccelStepper` library using the Library Manager.
   - Upload the code to the Arduino MEGA.

## Usage

1. **Start the Program**:
   - Open the serial monitor in the Arduino IDE.
   - Set the baud rate to 4800.
   
2. **Select a Motion Experience**:
   - Follow the prompts in the serial monitor to select a motion experience.
   - The available options are:
     1. Move in one direction.
     2. Full experience of motion.
     3. Roller coaster simulation.
   
3. **Customize Parameters**:
   - For the single-direction movement, follow the prompts to choose the axis, speed, acceleration, and position.
   - For the other experiences, the motion profiles are predefined.

4. **Reset**:
   - After completing an experience, you can select a new one or reset the chair to its initial position.

## Contributors

This project was developed by:
- Matthew Coffey
- John Cook
- Sean Schlief
- Charles Zipperer
