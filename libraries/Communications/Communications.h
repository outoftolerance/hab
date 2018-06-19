#ifndef Communications_h
#define Communications_h

#include <Telemetry.h>

/**
 * @brief      Class for communications outputs
 */
class Communications
{
	private:
		HardwareSerial* output_0_; /**< Pointer to hardware serial port for output 0 */
		HardwareSerial* output_1_; /**< Pointer to hardware serial port for output 1 */
	public:
		/**
		 * @brief      Default constructor
		 */
		Communications();

		/**
		 * @brief      Constructor with required inputs
		 *
		 * @param      terminal_0  Pointer to output 0
		 * @param      terminal_1  Pointer to output 1
		 */
		Communications(HardwareSerial* terminal_0, HardwareSerial* terminal_1);

		/**
		 * @brief      Sends communications to all outputs
		 */
		void send(TelemetryStruct* telemetry);
}

#endif