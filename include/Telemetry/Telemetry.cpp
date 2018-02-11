#include "Telemetry.h"

void Telemetry::initGps(Stream* serial)
{
	gps_serial = serial;
}

void Telemetry::read()
{
	readGps();
	readAccel();
	readGyro();
	readMag();
	readPressure();
}

void Telemetry::readGps()
{
	while(gps_serial->available())
	{
		gps.encode(serial->read());
	}
}