# HAB
HAB is a High Altitude Balloon project designed with specific goals in mind.
 - Take a picture of the curvature of the earth
 - Take a picture of the earth and space
 - Learn new things through project research (atmosphere, weather, etc)
 - Develop engineering skills (Software and Electrical)

# Mission States
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
