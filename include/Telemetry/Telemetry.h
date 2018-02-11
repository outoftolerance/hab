#ifndef Telemetry_h
#define Telemetry_h

#include "../TinyGPS++/TinyGPS++.h"

typedef struct AxisData
{
	int x;
	int y;
	int z;
} AxisData;

class Telemetry
{
	private:
		TinyGPSPlus gps;
		Stream* gps_serial
		void readGps(Stream* serial, TinyGPSPlus* gps);

		void readMag();

		void readAccel();

		void readGyro();

		void readPressure();

	public:
		//Telemetry wide
		void read();

		//GPS
		void initGps(Stream* serial);

		//Accelerometer
		void getAccelRaw();
		void getAttitude();

		//Gyroscope
		void getGyroRaw();

		//Magnetometer
		void getMagRaw();

		//Pressure
		void getPressureRaw();
};

#endif