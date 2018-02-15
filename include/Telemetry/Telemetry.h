#ifndef Telemetry_h
#define Telemetry_h

#include "../TinyGPS++/TinyGPS++.h"


typedef struct AxisData
{
	int x;
	int y;
	int z;
} AxisData;

typedef struct TelemetryStatus
{
	bool gps;
	bool accel;
	bool gyro;
	bool mag;
	bool pressure;
} TelemetryStatus;

class Telemetry
{
	private:
		//Telemtry wide
		void init();

		//GPS
		TinyGPSPlus _gps;
		Stream* _gps_serial;
		void initGps(Stream* serial);
		void updateGps();

		//Accelerometer
		void updateAccel();

		//Gyroscope
		void updateGyro();

		//Magnetometer
		void updateMag();

		//Pressure
		void updatePressure();

	public:
		//Telemetry wide
		Telemetry(Stream* gps_stream);
		void update();
		TelemetryStatus status;

		//Accelerometer
		AxisData getAccelRaw();
		AxisData getAttitude();

		//Gyroscope
		AxisData getGyroRaw();

		//Magnetometer
		AxisData getMagRaw();

		//Pressure
		float getPressureRaw();
};

#endif