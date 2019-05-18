# HAB
HAB is a High Altitude Balloon project designed with specific goals in mind.
 - Take a picture of the curvature of the earth
 - Take a picture of the earth and space
 - Learn new things through project research (atmosphere, weather, etc)
 - Develop engineering skills (Software and Electrical)

The code for this project is written in the Arduino language so that it is portable and easy to understand for other people who might want to create a similar project of their own, or extend the project in their own way.

# Getting Started
In order to get started with this software, please follow the steps outlined below. This guide will walk through the various elements of the project and will direct you to other resources along the way (such as hardware design documents).

## Hardware
Check out the hardware designs at [Google Drive](https://drive.google.com/drive/folders/1r9GAZf073PXg80UJds1Ubpu39Qg9nagI?usp=sharing).

## Software

1. Install the Arduino IDE, this can be found at the [Arduino website](http://arduino.cc).

2. Clone this repository to your computer, below are the commands to do this in Linux.
```bash
cd ~/Documents
git clone https://github.com/outoftolerance/hab
```

3. Start the Arduino IDE.

4. Change the path for the library folder to reference the project folder at `~/Documents/hab`, this is needed for some of the libraries to work correctly. To do this go to `File -> Preferences` and alter the "Sketchbook Location" field, then click ok.

5. Select the Arduino board type you are using from `Tools -> Board`.

6. Click `Upload` to program the board!

# Mission States
In order to control the different functions of the balloon at different stages in the flight, a state machine was developed which tracks mission state based on inputs such as telemetry, button presses, and command messages.

The following states have been defined in the state machine, for a more graphical view see the [Software Design Documentation](https://drive.google.com/file/d/1JTLx4fkZP5AwBsL-5_mE49o71bBY4H_E/view?usp=sharing) on Google Drive.

`Staging` <-> `Takeoff` -> `Ascending` -> `Descending` -> `Landing` -> `Recovery` <-> `Recovered`

## Staging
While waiting to launch the missing and on the ground, this state is known as `Staging` and is used to prepare the vehicle and sub-systems.

### Configuration
Telemetry
 - Update Frequency: 1 Hz
 - Reporting Frequency: 1/5 Hz
 - Logging Frequency: 1/5 Hz
Position
 - Reporting Frequency: 1/30 Hz
Indicators
 - LED: On
 - Beeper: Off

### Entrance Routes
- State is set by default at boot.
- Launch switch is flipped to staging and current mission state is `Takeoff`.

### Exit Routes
- Launch switch is flipped to `Takeoff` from `Staging`.

## Takeoff
Once ready to takeoff and all staging work has been completed this state is engaged manually.

### Configuration
Telemetry
 - Update Frequency: 1 Hz
 - Reporting Frequency: 1/15 Hz
 - Logging Frequency: 1/5 Hz
Position
 - Reporting Frequency: 1/15 Hz
Indicators
 - LED: On
 - Beeper: On

### Entrance Routes
- Launch switch is flipped to `Takeoff` from `Staging`.

### Exit Routes
- Launch switch is flipped to `Staging` from `Takeoff`.
- Altitude increases above 50m.

## Ascending
Once takeoff has happened, the ascending state is automatically engaged when ascension is detected by the telemetry sensors.

### Configuration
Telemetry
 - Update Frequency: 1 Hz
 - Reporting Frequency: 1/60 Hz
 - Logging Frequency: 1/5 Hz
Position
 - Reporting Frequency: 1/30 Hz
Indicators
 - LED: Off
 - Beeper: Off

### Entrance Routes
- Altimeter detects altitude rises above 500m.

### Exit Routes
- Accelerometer detects freefall and/or altitude starts dropping for 5 consecutive seconds.

## Descending
Once ascension has completed, the descending state is automatically engaged when descent is detected. This will occur when the balloon has popped at the highest point during the flight.

### Configuration
Telemetry
 - Update Frequency: 1 Hz
 - Reporting Frequency: 1/60 Hz
 - Logging Frequency: 1/5 Hz
Position
 - Reporting Frequency: 1/30 Hz
Indicators
 - LED: Off
 - Beeper: Off

### Entrance Routes
- Accelerometer detects freefall and/or altitude starts dropping for 5 consecutive seconds.

### Exit Routes
- When the altitude is detected to be below 500m.

## Landing
Once descending to terminal altitude this state is automatically engaged.

### Configuration
Telemetry
 - Update Frequency: 1 Hz
 - Reporting Frequency: 1/60 Hz
 - Logging Frequency: 1/5 Hz
Position
 - Reporting Frequency: 1/5 Hz
Indicators
 - LED: On
 - Beeper: On

### Entrance Routes
- When the altitude is detected to be below 500m.

### Exit Routes
- When touchdown has been detected by the acceleromter, or altitude stops decreasing for 5 consecutive seconds.

## Recovery
This state is automatically engaged once touchdown has been detected.

### Configuration
Telemetry
 - Update Frequency: 1 Hz
 - Reporting Frequency: 1/300 Hz
 - Logging Frequency: 1/5 Hz
Position
 - Reporting Frequency: 1/300 Hz
Indicators
 - LED: On
 - Beeper: On

### Entrance Routes
- When touchdown has been detected by the acceleromter, or altitude stops decreasing for 5 consecutive seconds.
- After waiting 5 minutes in silsnce mode without being powered off.

### Exit Routes
- When the Silence button has been pressed for 1 second.
