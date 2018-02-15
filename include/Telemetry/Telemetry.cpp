#include "Telemetry.h"

/**
 * @brief      Constructs the Telemetry object and performs initialisation operations
 */
Telemetry::Telemetry(Stream* gps_serial)
{
	init();
	initGps(gps_serial);
}

/**
 * @brief      Initialises all variables and objects to their default state.
 */
void Telemetry::init()
{
	//Initialise the status object
	status.gps = FALSE;
	status.accel = FALSE;
	status.gyro = FALSE;
	status.mag = FALSE;
	status.pressure = FALSE;
}

/**
 * @brief      initGps function passes in a Stream object to be used by the GPS object
 *
 * @param      serial  The Stream object for the serial port the GPS is attached to
 */
void Telemetry::initGps(Stream* gps_serial)
{
	_gps_serial = gps_serial;
	_gps_serial->begin(9600);
}

/**
 * @brief      Performs a read on each sensor managed by the Telemetry object
 */
void Telemetry::update()
{
	updateGps();
	updateAccel();
	updateGyro();
	updateMag();
	updatePressure();
}

/**
 * @brief      Reads latest data from the GPS receiver
 */
void Telemetry::updatesGps()
{
	while(_gps_serial->available())
	{
		_gps.encode(_gps_serial->read());
	}
}

/**
 * @brief      Reads latest data from the Accelerometer
 */
void Telemetry::updatesAccel()
{

}

/**
 * @brief      Reads latest data from the Gyroscope
 */
void Telemetry::updatesGyro()
{

}

/**
 * @brief      Reads latest data from the Magnetometer
 */
void Telemetry::updatesMag()
{

}

/**
 * @brief      Reads latest data from the Pressure sensor
 */
void Telemetry::readPressure()
{

}