#include "Telemetry.h"

/*------------------------------Constructor Methods------------------------------*/
Telemetry::Telemetry(Stream& gps_serial, int gps_fix_pin) :
	gps_serial_(gps_serial),
	gps_fix_pin_(gps_fix_pin)
{
	gps_serial_buffer_ = new Buffer(GPS_SERIAL_BUFFER_SIZE);
	gps_fix_status_ = false;
}

/*------------------------------Private Methods------------------------------*/

bool Telemetry::init()
{
	//Initialise the GPS
	static_cast<HardwareSerial&>(gps_serial_).begin(GPS_SERIAL_BAUD);

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

	//Everything initialized correctly
	return true;
}

void Telemetry::update_()
{
	updateGps_();
	updateAccelerometer_();
	updateGyroscope_();
	updateMagnetometer_();
	updateBarometer_();
}

void Telemetry::updateGps_()
{
	char c;

	while(gps_serial_.available())
	{
		c = gps_serial_.read();
		gps_.encode(c);
		gps_serial_buffer_->push(c);
	}

	gps_fix_status_ = digitalRead(gps_fix_pin_);
}

void Telemetry::updateAccelerometer_()
{
	accelerometer_.getEvent(&accelerometer_data_);
}

void Telemetry::updateGyroscope_()
{
	//gyroscope_.getEvent(&gyroscope_data_);
}

void Telemetry::updateMagnetometer_()
{
	magnetometer_.getEvent(&magnetometer_data_);
}

void Telemetry::updateBarometer_()
{
	barometer_.getEvent(&barometer_data_);
}

/*------------------------------Public Methods------------------------------*/

bool Telemetry::get(TelemetryStruct& telemetry)
{
	//Update all sensor data
	update_();

	//Calculate attitude from accelerometer
	if (!sensor_board_.accelGetOrientation(&accelerometer_data_, &orientation_))
	{
		return false;
	}

	//Correct magnetometer values based on tilt
	if (!sensor_board_.magTiltCompensation(SENSOR_AXIS_Z, &magnetometer_data_, &accelerometer_data_))
	{
	  return false;
	}

	//Calculate heading from magnetometer
	if (!sensor_board_.magGetOrientation(SENSOR_AXIS_Z, &magnetometer_data_, &orientation_))
	{
		return false;
	}

	//Calculate altitude from barometer
	if (!barometer_data_.pressure)
	{
		return false;
	}

	//Get ambient temperature in C
	float temperature;
	barometer_.getTemperature(&temperature);

	//Convert atmospheric pressure, SLP and temp to altitude
	float altitude_barometric;
	altitude_barometric = barometer_.pressureToAltitude((float)SENSORS_PRESSURE_SEALEVELHPA, barometer_data_.pressure, temperature);

	//Assign to output struct
	telemetry.latitude = (float)gps_.location.lat();
	telemetry.longitude = (float)gps_.location.lng();
	telemetry.altitude = (float)gps_.altitude.meters();
	telemetry.altitude_barometric = altitude_barometric;
	telemetry.roll = orientation_.roll;
	telemetry.pitch = orientation_.pitch;
	telemetry.heading = orientation_.heading;
	telemetry.course = (float)gps_.course.deg();
	telemetry.temperature = temperature;
	telemetry.pressure = barometer_data_.pressure;

	return true;
}

bool Telemetry::getAccelerometerRaw(AxisData& accelerometer)
{
	return false;
}

bool Telemetry::getGyroscopeRaw(AxisData& gyroscope)
{
	return false;
}

bool Telemetry::getMagnetometerRaw(AxisData& magnetometer)
{
	return false;
}

bool Telemetry::getBarometerRaw(float& data)
{
	return false;
}

int Telemetry::getGpsString(char string[])
{
	int i = 0;

	while(gps_serial_buffer_->available())
	{
		string[i] = gps_serial_buffer_->pop();
		i++;
	}

	return i;
}

bool Telemetry::getGpsFixStatus()
{
	return gps_fix_status_;
}

float Telemetry::getGpsHdop()
{
	return (float)gps_.hdop.hdop();
}

long Telemetry::getGpsDateTime()
{
	return 0;
}