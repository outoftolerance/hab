#ifndef Log_h
#define Log_h

#include <Arduino.h>

/**
 * @brief      Logging class
 * @details    Logs data according to different log levels to output stream
 */
class Log {
	private:
		Stream& output_; /**< Pointer to stream used for output */
		bool debug_; /**< Debug mode flag */
	public:
		/**
		 * @brief      Log default constructor
		 */
		Log();

		/**
		* @brief      Logger actual constructor
		* @param      terminal  Stream pointer to output stream used
		* @param[in]  debug     Boolean to activate debug mode
		*/
		Log(Stream& terminal, bool debug);

		/**
		* @brief      Infomation log level
		* @param      message  The message to be logged
		*/
		void info(const char message[]);

		/**
		* @brief      Debug log level
		* @param      message  The message to be logged
		*/
		void debug(const char message[]);

		/**
		* @brief      Error log level
		* @param      message  The message to be logged
		*/
		void error(const char message[]);

		/**
		* @brief      Fatal log level
		* @param      message  The message to be logged
		*/
		void fatal(const char message[]);
};

#endif
