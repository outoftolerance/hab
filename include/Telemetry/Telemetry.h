#ifndef Telemetry_h
#define Telemetry_h

#include <Wire.h>
#include "../TinyGPS++/TinyGPS++.h"
#include "../Adafruit_Sensor/Adafruit_Sensor.h"
#include "../Adafruit_LSM303DLHC/Adafruit_LSM303_U.h"
#include "../Adafruit_BMP085_Unified/Adafruit_BMP085_U.h"
#include "../Adafruit_L3GD20_U/Adafruit_L3GD20_U.h"
#include "../Adafruit_10DOF/Adafruit_10DOF.h"

/**
 * Structure for axis related data (e.g. acceleration, velocity, gyro, mag, etc...)
 */
typedef struct AxisData
{
	float x;
	float y;
	float z;
} AxisData;

/**
 * Structure for complete telemetry output.
 */
typedef struct TelemetryStruct
{
	float lattitude; /**< Lattitude in decimal degrees */
	float longitude; /**< Longitude in decimal degrees */
	float roll; /**< Roll in radians */
	float pitch; /**< Pitch in radians */
	float heading; /**< Magnetic heading in degrees */
	float altitude; /**< Altitude in meters from GPS */
	float altitude_barometric;  /**< Altitude in meters from barometer */
	float temperature; /**< Temperature in degrees C */
	float pressure; /**< Pressure in pascals */
} TelemetryStruct;

/**
 * @brief      Telemetry class definition. Class interacts with all sensors and gives access to sensor data in a useful way
 */
class Telemetry
{
	private:
		/**
		 * @brief      Updates all sensors
		 */
		void update_();

		/**
		 * @brief      Updates with the latest GPS data
		 */
		void updateGps_();

		/**
		 * @brief      Updates with the latest accelerometer data
		 */
		void updateAccel_();

		/**
		 * @brief      Updates with the latest gyroscope data
		 */
		void updateGyro_();

		/**
		 * @brief      Updates with the latest magnetometer data
		 */
		void updateMag_();

		/**
		 * @brief      Updates with the latest baronmeter data
		 */
		void updatePressure_();

		TinyGPSPlus gps_;	//Defines Tiny GPS object
		Stream* gps_serial_;	//Defines Stream object for GPS device serial port

		Adafruit_10DOF sensor_board_ = Adafruit_10DOF(); /**< 10 degree of freedom sensor board */
		Adafruit_LSM303_Accel_Unified accelerometer_ = Adafruit_LSM303_Accel_Unified(30301); /**< accelerometer/gyroscope private object */
		Adafruit_LSM303_Mag_Unified magnetometer_ = Adafruit_LSM303_Mag_Unified(30302); /**< magnetometer private object */
		Adafruit_BMP085_Unified barometer_ = Adafruit_BMP085_Unified(18001); /**< barometer private object */

		sensors_event_t accelerometer_data_; /**< Struct for latest accelerometer data */
		sensors_event_t magnetometer_data_; /**< Struct for latest magenetometer data */
		sensors_event_t barometer_data_; /**< Struct for latest barometer data */
		sensors_vec_t orientation_; /**< Orientation struct needed by Adafruit sensor library for some functions */

	public:
		/**
		 * @brief      Default telemetry class constructor, not used
		 */
		Telemetry();

		/**
		 * @brief      Telemetry class constructor
		 *
		 * @param      gps_stream  Pointer to the Stream object for the GPS serial port
		 */
		Telemetry(Stream* gps_stream);

		/**
		 * @brief      Initialises all variables and objects to their default value/state
		 */
		bool init();

		/**
		 * @brief      Returns current telemetry of system
		 * @param      Pointer to variable to output data to
		 * @return     Boolean success/fail indicator
		 */
		bool get(TelemetryStruct* telemetry);

		/**
		 * @brief      Gets raw accelerometer data
		 * @param      Pointer to variable to output data to
		 * @return     Boolean success/fail indicator
		 */
		bool getAccelerometerRaw(AxisData* accelerometer);

		/**
		 * @brief      Gets the raw gyroscope data
		 * @param      Pointer to variable to output data to
		 * @return     Boolean success/fail indicator
		 */
		bool getGyroscopeRaw(AxisData* gyroscope);

		/**
		 * @brief      Gets the raw magnetometer data
		 * @param      Pointer to variable to output data to
		 * @return     Boolean success/fail indicator
		 */
		bool getMagnetometerRaw(AxisData* magnetometer);

		/**
		 * @brief      Gets the raw barometer data
		 * @param      Pointer to variable to output data to
		 * @return     Boolean success/fail indicator
		 */
		bool getBarometerRaw(AxisData* barometer);
};

#endif
