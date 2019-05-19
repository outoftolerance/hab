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

# Hardware
This project makes use of the `ATSAMD21G18A-U` Microcontroller, this is supported through the Arduino project as the Arduino Zero and through Adafruit as their Feather M0 board. We make use of the Adafruit version of the board support package as it allows more flexibility in terms of serial ports as will be explained in detail further in this section.

## Why This Micro?
We chose this specific microcontroller because it has a feature called SERCOM that allows for in-software definition of serial, I2C, and SPI interfaces using different pins. Since we have a large number of serial peripherals for this project (GPS, Radio, Cellular) we needed to be able to define more than the standard number of serial ports for an Arduino board.

Additionally, the chip is not very large. We had the option of using the `ATMEGA2560` as used on the Arduino Mega board, however the physical size (and cost!) of that microcontroller is significantly larger in comarison. 

We also get other side benefits such as more program memory, and faster clock speed, that aren't NEEDED for this project but are a nice to have.

## SERCOM
We make use of the SERCOM feature of the micro by definig two additional serial ports, no other use of the SERCOM system is needed for this project. 

*Note that when adding serial port interfaces using SERCOM, the TX pin may only reside on either PAD0 or PAD2 of a specific SERCOM interface! This limits the available SERCOM interfaces that we can choose from because some pads are being used for other interfaces (like I2C or SPI).*

The following table shows the pinout for the `ATSAMD21G18A-U` SERCOM and the uses that we are making of these pins for the project.

|Microcontroller Pin|Arduino 'Pin'|SERCOM|SERCOM alt|Configured Interface|
|---|---|---|---|--|
|PA00|Used by xtal|SERCOM1.0|||
|PA01|Used by xtal|SERCOM1.1|||
|PB08|A1|SERCOM4.0|||
|PB09|A2|SERCOM4.1|||
|PA04|A3|SERCOM0.0|||
|PA05|A4|SERCOM0.1|||
|PA06|D8|SERCOM0.2|||
|PA07|D9|SERCOM0.3|||
|PA08|D4|SERCOM0.0|SERCOM2.0|Serial3 - Cellular|
|PA09|D3|SERCOM0.1|SERCOM2.1|Serial3 - Cellular|
|PA10|D1|SERCOM0.2|SERCOM2.2|Serial1 - GPS|
|PA11|D0|SERCOM0.3|SERCOM2.3|Serial1 - GPS|
|PB10|D23 / MOSI|SERCOM4.2|||
|PB11|D24 / SCK|SERCOM4.3|||
|PA12|D22 / MISO|SERCOM2.0|SERCOM4.0||
|PA13|Used by EDBC|SERCOM2.1|SERCOM4.1||
|PA14|D2|SERCOM2.2|SERCOM4.2||
|PA15|D5|SERCOM2.3|SERCOM4.3||
|PA16|D11|SERCOM1.0|SERCOM3.0|Serial2 - Radio|
|PA17|D13|SERCOM1.1|SERCOM3.1||
|PA18|D10|SERCOM1.2|SERCOM3.2|Serial2 - Radio|
|PA19|D12|SERCOM1.3|SERCOM3.3||
|PA20|D6|SERCOM5.2|SERCOM3.2||
|PA21|D7|SERCOM5.3|SERCOM3.3||
|PA22|D20 / SDA|SERCOM3.0|SERCOM5.0||
|PA23|D21 / SCL|SERCOM3.1|SERCOM5.1||
|PA24|Used by USB|SERCOM3.2|SERCOM5.2|Serial - Debug|
|PA25|Used by USB|SERCOM3.3|SERCOM5.3|Serial - Debug|
|PB22|D30 / EDBG TX|SERCOM5.2||Serial5 - Unused|
|PB23|D31 / EDBG RX|SERCOM5.3||Serial5 - Unused|
|PA30|Used by SWCLK|SERCOM1.2|||
|PA31|Used by SWDIO|SERCOM1.3|||
|PB02|A5|SERCOM5.0|||
|PB03|D25 / RX LED|SERCOM5.1|||

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
