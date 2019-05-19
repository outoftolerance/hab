# HAB
HAB is a High Altitude Balloon project designed with specific goals in mind.
 - Take a picture of the curvature of the earth
 - Take a picture of the earth and space
 - Learn new things through project research (atmosphere, weather, etc)
 - Develop engineering skills (Software and Electrical)

The code for this project is written in the Arduino language so that it is portable and easy to understand for other people who might want to create a similar project of their own, or extend the project in their own way.

# Hardware


## Microcontroller
For this project we use the `ATSAMD21G18A-U` Microcontroller, this is supported through the Arduino project as the Arduino Zero and through Adafruit as their Feather M0 board (as well as other variants available). We make use of the Adafruit version of the board support package as it allows more flexibility in terms of serial ports by not using the EDBG interface that is by default used by the Arduino Zero, instead it uses the USB interface.

We chose this specific microcontroller because it has a feature called SERCOM that allows for in-software definition of serial, I2C, and SPI interfaces using different pins. Since we have a large number of serial peripherals for this project (GPS, Radio, Cellular, APRS) we needed to be able to define more than the standard single serial port for an Arduino board. While it would be possible to define additional serial interfaces used the `SoftwareSerial` library, this is generally not good practice and has many implementation pitfalls.

Additionally, the chip is not very large. We had the option of using the `ATMEGA2560` (as used on the Arduino Mega board, which has four serial interfaces) however the physical size (and cost!) of that microcontroller is significantly larger in comarison. We also get other side benefits such as more program memory, and faster clock speed, that aren't NEEDED for this project but are a nice to have.

### SERCOM
We make use of the SERCOM feature of the micro by definig two additional serial ports beyond those already defined by default, no other customised use of the SERCOM system is needed for this project.

*Note that when adding serial port interfaces using SERCOM, the TX pin may only reside on either PAD0 or PAD2 of a specific SERCOM interface! This limits the available SERCOM interfaces that we can choose from because some pads are being used for other interfaces (like I2C or SPI).*

The following table shows the pinout for the `ATSAMD21G18A-U` SERCOM and the uses that we are making of these pins for the project. This table was sourced from ADafruit's fantastic guide on using the Feather M0 that can be [found on their website](https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/overview).

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
|PB10|D23 / MOSI|SERCOM4.2||SPI - MicroSD|
|PB11|D24 / SCK|SERCOM4.3||SPI - MicroSD|
|PA12|D22 / MISO|SERCOM2.0|SERCOM4.0|SPI - MicroSD|
|PA13|Used by EDBC|SERCOM2.1|SERCOM4.1||
|PA14|D2|SERCOM2.2|SERCOM4.2||
|PA15|D5|SERCOM2.3|SERCOM4.3||
|PA16|D11|SERCOM1.0|SERCOM3.0|Serial2 - Radio|
|PA17|D13|SERCOM1.1|SERCOM3.1||
|PA18|D10|SERCOM1.2|SERCOM3.2|Serial2 - Radio|
|PA19|D12|SERCOM1.3|SERCOM3.3||
|PA20|D6|SERCOM5.2|SERCOM3.2||
|PA21|D7|SERCOM5.3|SERCOM3.3||
|PA22|D20 / SDA|SERCOM3.0|SERCOM5.0|I2C - Sensors|
|PA23|D21 / SCL|SERCOM3.1|SERCOM5.1|I2C - Sensors|
|PA24|Used by USB|SERCOM3.2|SERCOM5.2|Serial - Debug|
|PA25|Used by USB|SERCOM3.3|SERCOM5.3|Serial - Debug|
|PB22|D30 / EDBG TX|SERCOM5.2||Serial5 - APRS|
|PB23|D31 / EDBG RX|SERCOM5.3||Serial5 - APRS|
|PA30|Used by SWCLK|SERCOM1.2|||
|PA31|Used by SWDIO|SERCOM1.3|||
|PB02|A5|SERCOM5.0|||
|PB03|D25 / RX LED|SERCOM5.1|||

## Communications
Communications is one of the major difficulties when designing a High Altitude Balloon because of the distances that it has to travel and the fact that the system needs to be recovered at a random, unknown, future location (or you lose all your hard work!). For this reason, there are multiple redundant communications interfaces that have been designed into the vehicle.

### Packet Radio
The first, and simplest, form of communication between the vehicle and us on the ground is a packet radio. Essentially this works as a serial port over radio waves and allows data to be passed from the vehicle to the ground (such as position information) as well as allowing commands to be send to the vehicle during operation.

We chose an [RFD900+ radio modem](http://store.rfdesign.com.au/rfd-900p-modem/) for this project, it is well known within the drone industry as a reliable and well-performing radio that can be configured and paired with different antenna systems to provide exceptional range.

This communications interface will be used to communicate the following information:
- Telemetry from vehicle
- Position from vehicle
- Events from vehicle
- Commands to vehicle

### Cellular
Secondly we will use a cellular modem to send and receive data from the ground to the vehicle. This method is not reliable as a tracking system for the vehicle throughtout the flight, however is robust at low altitudes within cellular network reception range. It is also a flexible interface, it can either send text messages to us, or data over the internet, or a combination of both!

The reason we added this radio was as a backup in-case the other tracking systems fail, or when at low altitudes, radio signals from the other two methods are obscured by terrain or ground objects where the balloon lands. The cellular connection should be able to text message us with the location of the vehicle at low altitudes and when at rest.

The communucations interface will be used to communicate the following information:
- Position from the vehicle
- Events from the vehicle

### APRS
An APRS (Automatic Packet Reporting System) radio has also been added, this allows for the tracking of the balloon through the HAM radio APRS network of receivers, and potentially from our own receiver. The system is very limited in terms of how frequently data can be transmitted and how much data can be transmitted at a time, however it is very very long range and is well supported as a method of tracking vehicles. 

We chose the [MicroTrak 2000](https://drive.google.com/open?id=1uPaUfZ47ibVQPEURd_sj6lUmIUkslmQF) APRS transmitter from Byonics for this project because of its small form-factor, high power, and easy integration into our custom electical and mechanical design. This model is no longer in production and has been superceded by an even smaller form-factor device, the MicroTrak 2001.

This communications interface will be used to communicate the following information:
- Position from the vehicle

## Sensors
There are some sensors needed for basic functionality of the balloon (such as a GPS receiver) and others are just for fun!

### GPS
A GPS receiver is a basic requirement for being able to position the vehicle and track it during its mission. We chose a basic off-the-shelf GPS receiver from Adafruit that has been used in high altitude balloon projects with success in the past, the [Adafruit GPS V3](https://www.adafruit.com/product/746).

### IMU
An IMU is added to give basic orentation information, as well as being able to detect freefall and other events such as touchdown. We chose a simple all-in-one IMU board, the [Adafruit 10DOF](https://www.adafruit.com/product/1604). This is no longer available but still functions great and we already had one.

### Atmospheric
None of these sensors are really neded, however the data they produce is fun to look at after the flight and get an idea of the make-up of our atmosphere! We chose to add temperature, humidity, and gas sensors to the project, the specific models and boards haven't been defined yet but will come.

## Interconnects
The different elements of the system are connected together using Serial, I2C, and SPI interfaces, as well as a few GPIO used for some smaller peripherals. The folliwing diagram shows how each element of the system is connected.

![Hardware Interface Connection Diagram](documents/hardware_interface_connection_diagram.png?raw=true)

# Software
The software for this project is likely to be much more complex than is really required for such a simple mission, however part of the goal of this project was to learn new techniques and to advance my knowledge of object-oriented programming, abstraction, division of responsibility, and writing extensible and maintainable code. 

For this reason different sub-systems are built into their own libraries, these libraries are described at a functional level here, and at a technical level in the header file comments for each package.

## Mission States
In order to control the different functions of the balloon at different stages in the flight, a state machine was developed which tracks mission state based on inputs such as telemetry, button presses, and command messages.

![Mission State Diagram](documents/software_mission_state_diagram.png?raw=true)

### State Descriptions

**Staging:** While waiting to launch and on the ground, used to prepare the vehicle and sub-systems.

**Takeoff:** Once ready to takeoff and all staging work has been completed this state is engaged manually.

**Ascending:** Once takeoff has completed and the vehicle is ascending in altitude.

**Descending:** Once ascension has completed and descent has started.

**Landing:** When vehicle is about to land and has descended almost to the ground.

**Receovery:** Once touchdown has been detected and the vehicle is waiting for recovery.

**Receovered:** When recovery has been detected but power-off is pending.

### State Table

|State|Configuration|Entrance Condititions|Exit Conditions|
|-----|-------------|---------------------|---------------|
|Staging   |<ul><li>Telemetry Reporting Frequency: 1/5 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/30 Hz</li><li>LEDs: On</li><li>Beeper: Off</li></ul>|<ul><li>State is set by default at boot.</li><li>Launch switch is flipped to staging and current mission state is `Takeoff`.</li></ul>|<ul><li>Launch switch is flipped to `Takeoff` from `Staging`.</li></ul>|
|Takeoff   |<ul><li>Telemetry Reporting Frequency: 1/15 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/15 Hz</li><li>LEDs: On</li><li>Beeper: On</li></ul>|<ul><li>Launch switch is flipped to `Takeoff` from `Staging`.</li></ul>|<ul><li>Launch switch is flipped to `Staging` from `Takeoff`.</li><li>Altitude increases above 500m.</li></ul>|
|Ascending |<ul><li>Telemetry Reporting Frequency: 1/60 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/30 Hz</li><li>LEDs: Off</li><li>Beeper: Off</li></ul>|<ul><li>Altitude increases above 500m.</li></ul>|<ul><li>Accelerometer detects freefall or altitude starts dropping for 5 consecutive seconds.</li></ul>|
|Descending|<ul><li>Telemetry Reporting Frequency: 1/60 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/30 Hz</li><li>LEDs: Off</li><li>Beeper: Off</li></ul>|<ul><li>Accelerometer detects freefall or altitude starts dropping for 5 consecutive seconds.</li></ul>|<ul><li>Altitude decreases below 500m.</li></ul>|
|Landing   |<ul><li>Telemetry Reporting Frequency: 1/60 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/5 Hz</li><li>LEDs: On</li><li>Beeper: On</li></ul>|<ul><li>Altitude decreases below 500m.</li></ul>|<ul><li>Accelerometer stops detecting motion or altitude stops changing for 5 consecutive seconds.</li></ul>|
|Recovery  |<ul><li>Telemetry Reporting Frequency: 1/300 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/300 Hz</li><li>LEDs: On</li><li>Beeper: On</li></ul>|<ul><li>Accelerometer stops detecting motion or altitude stops changing for 5 consecutive seconds.</li><li>After waiting 5 minutes in Recovered mode.</li></ul>|<ul><li>Silence button is pressed.</li></ul>|
|Recovered |<ul><li>Telemetry Reporting Frequency: 1/300 Hz</li><li>Telemetry Logging Frequency: 1/5 Hz</li><li>Position Reporting Frequency: 1/300 Hz</li><li>LEDs: Off</li><li>Beeper: Off</li></ul>|<ul><li>Silence button is pressed.</li></ul>|<ul><li>After waiting 5 minutes in Recovered mode.</li></ul>|

## Buffer

## Timer
The Timer library provides basic timing functionaly, this is used to create time-outs for different reasons within the code. It allows the main program loop to run really fast even when there are tasks that might take minutes or even hours without having to wait for these things to complete by using delays (the basic Arduino way). 

This is essentially a primitive way of "multi-threading" on a microcontroller as many tasks can be done "in parallel", i.e. the execution of different elements of different tasks can be done asynchronously.

## Log
The Log library provides a simple interface for event logging withing the program. This allows for easy debugging of the software and provides many useful features such as log levels.

## Data Log
The Data Log librariy provides a way to log complex information to a file on an SD card. This could be telemetry information or other state information that is sensed during the flight of the vehicle. These logs can then be read back post-flight for analysis.

## Telemetry
The Telemetry library provides a level of abstraction from the sensors that are being used to get telemetry information (GPS, IMU, etc) and provides the latest telemetry in an easily digestible manner for higher level applications.

## Simple HDLC
The Simple HDLC (High-level Data Link Control) library provides basic messaging functionaly including message serialization, deserialization, CRC checking, etc. This wraps different byte-wise interfaces (currently implemented only on serial ports) to create a generic message interface.

## Simple Message Protocol
The Simple Message Protocol library (SMP) wraps the SimpleHDLC library at an even higher level, by pre-defining different types of messages and providing encoding and decoding functions for those messages. This library can be thought of as defining a dialect or language between the microcontroller and any other external systems it is messaging with.

## External Libraries
The project also depends on a few externally developed libraries as listed here.

- TinyGPS++
- Adafruit Sensor
- Adafruit 10DOF
- Adafruit BMP085
- Adafruit L3GD20
- Adafruit LSM303DLHC

