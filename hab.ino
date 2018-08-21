#include <Telemetry.h>
#include <Log.h>

const bool debug = false; /**< Global debug flag, changes behaviour and outputs */

Log logger(&Serial, debug); /**< Log object */

Telemetry telemetry(&Serial1); /**< Telemetry object */
TelemetryStruct current_telemetry; /**< Struct to store current telemetry data */

/**
 * @brief Initialises main program elements
 */
void initialise();

/**
 * @brief System setup function
 * @details Initialises all system componenets at start-up
 */
void setup() {
  //Start debug serial port
  Serial.begin(57600);
  logger.info("HAB systems starting...");

  //Initialise the telemetry system
  logger.info("Initialising telemetry subsystem...");
  if(!telemetry.init())
  {
    logger.fatal("Failed to initialise telemetry subsystem!");
    while(1);
  }
  logger.info("Telemetry initialised successfully!");
}

/**
 * @brief Main program loop
 * @details Called after setup() function, loops inifiteley, everything happens here
 */
void loop() {
  //Get latest telemetry data
  logger.debug("Updating telemetry data...");
  if(!telemetry.get(&current_telemetry))
  {
    logger.error("Telemetry was not able to be read.");
  }
  logger.debug("Telemetry data updated.");

  //Send location to APRS tracking


  //Send telemetry to SD card logger


  //Send telemetry to transmission radios


  //
}

void initialise()
{
  //Initialise the telemetry struct values
  current_telemetry.lattitude = 0;
  current_telemetry.longitude = 0;
  current_telemetry.roll = 0;
  current_telemetry.pitch = 0;
  current_telemetry.heading = 0;
  current_telemetry.altitude = 0;
  current_telemetry.altitude_barometric = 0;
  current_telemetry.temperature = 0;
  current_telemetry.pressure = 0;
}
