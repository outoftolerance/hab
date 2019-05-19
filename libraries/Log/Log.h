#ifndef Log_h
#define Log_h

#include <Arduino.h>

enum LOG_LEVELS
{
	DEBUG = 0,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

/**
 * @brief      Logging class
 * @details    Logs data according to different log levels to output stream
 */
class Log {
	public:
		/**
		* @brief      Logger actual constructor
		* @param      terminal  Stream pointer to output stream used
		* @param[in]  debug     Boolean to activate debug mode
		*/
		Log(Stream& terminal, LOG_LEVELS log_level);

		/**
		* @brief      Infomation log level
		* @param      message  The message to be logged
		*/
		void event(LOG_LEVELS level, const char message[]);

	private:
		Stream& output_;		/**< Reference to stream used for output */
		bool log_level_; 			/**< Debug mode flag */
};

#endif
