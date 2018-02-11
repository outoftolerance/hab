#include "include/Telemetry/Telemetry.h"

Telemetry telemetry;

//Define serial baud rates
const int serial_baud_0 = 9600;
const int serial_baud_1 = 9600;
const int serial_baud_2 = 9600;
const int serial_baud_3 = 9600;

void setup() {
  Serial.begin(serial_baud_0);
  Serial1.begin(serial_baud_1);
  Serial2.begin(serial_baud_2);
  Serial3.begin(serial_baud_3);

  telemetry.initGps(&Serial);
}

void loop() {
  
}
