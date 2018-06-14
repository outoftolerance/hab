#include "Telemetry.h"

/*------------------------------Constructor Methods------------------------------*/

Telemetry::Telemetry()
{

}

Telemetry::Telemetry(Stream* gps_serial)
{
	gps_serial_ = gps_serial;
}

/*------------------------------Private Methods------------------------------*/

bool Telemetry::init()
{
	//Initialise the status object
	status.gps = FALSE;
	status.accel = FALSE;
	status.gyro = FALSE;
	status.mag = FALSE;
	status.pressure = FALSE;

	//Initialise the GPS
	gps_serial_->begin(9600);

	//Initialise each sensor
	if(!accelerometer_.begin())
	{
		return false;
	}

	if(!magnetometer_.begin())
	{
		return false;
	}

	if(!barometer_.begin())
	{
		return false;
	}

	//Everything initialied correctly
	return true;
}

void Telemetry::update_()
{
	updateGps();
	updateAccel();
	updateGyro();
	updateMag();
	updatePressure();
}

void Telemetry::updatesGps_()
{
	while(_gps_serial->available())
	{
		_gps.encode(_gps_serial->read());
	}
}

void Telemetry::updateAccel_()
{
	accelerometer_.getEvent(&accelerometer_data_);
}

void Telemetry::updateGyro_()
{
	this.updateAccel();
}

void Telemetry::updateMag_()
{
	magnetometer_.getEvent(&magnetometer_data_);
}

void Telemetry::updatePressure_()
{
	barometer_.getEvent(&barometer_data_);
}

/*------------------------------Public Methods------------------------------*/

bool get(TelemetryStruct* telemetry)
{
	//Update all sensor data
	this.update();

	//Calculate attitude from accelerometer
	if (!sensor_board_.accelGetOrientation(&accelerometer_data_, &orientation_))
	{
		return false;
	}

	//Calculate heading from magnetometer
	if (!sensor_board_.magGetOrientation(SENSOR_AXIS_Z, &magnetometer_data_, &orientation_))
	{
		return false;
	}

	//Calculate altitude from barometer
	if (!barometer_event_.pressure)
	{
		return false;
	}

	//Get ambient temperature in C
	float temperature;
	barometer_.getTemperature(&temperature);

	//Convert atmospheric pressure, SLP and temp to altitude
	float altitude_barometric;
	altitude_barometric = barometer_.pressureToAltitude((float)SENSORS_PRESSURE_SEALEVELHPA, barometer_event_.pressure, temperature);

	//Assign to output struct
	telemetry->lattitude = (float)gps_.location.lat();
	telemetry->longitude = (float)gps_.location.lng();
	telemetry->roll = orientation_.roll;
	telemetry->pitch = orientation_.pitch;
	telemetry->heading = orientation_.heading;
	telemetry->altitude = (float)gps_.altitude.meters();
	telemetry->altitude_barometric = altitude_barometric;
	telemetry->temperature = temperature;
	telemetry->pressure = barometer_event_.pressure;

	return true;
}

bool getAccelerometerRaw(AxisData* accelerometer)
{
	return false;
}

bool getGyroscopeRaw(AxisData* gyroscope)
{
	return false;
}

bool getMagnetometerRaw(AxisData* magnetometer)
{
	return false;
}

bool getBarometerRaw(AxisData* barometer)
{
	return false;
}
