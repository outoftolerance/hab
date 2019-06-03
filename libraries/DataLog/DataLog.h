#ifndef DataLog_h
#define DataLog_h

#include <SPI.h>
#include <SD.h>

#define MAX_FILENAME_LENGTH 8
#define MAX_HEADER_LENGTH 128

/**
 * @brief      Logging class
 * @details    Logs data according to different log levels to output stream
 */
class DataLog {
	public:
		/**
		* @brief      Logger actual constructor
		* @param	  filename[]	The filename that should be logged to
		* @param	  header[]		The header that should be printed at the top of the file
		* @param	  chip_select 	The chip select pin for the SD card
		*/
		DataLog(int chip_select);

		/*
		 * @brief	  Initializes the logger by openning the file
		 */
		bool init(const String& filename, const String& header);

		/**
		 * @brief	  Logs a single line of data into the file.
		 * @param	  data 		Array of float data to log, each element will be comma separated
		 * @param	  size		Size of the array of data
		 * @param	  newline	If true, a newline will be printed after this data, false it won't
		 */
		bool entry(const float data[], int size, bool newline);

		/**
		 * @brief	  Logs a single line of data into the file.
		 * @param	  data 		Array of int data to log, each element will be comma separated
		 * @param	  size 		Size of the array of data
		 * @param	  newline	If true, a newline will be printed after this data, false it won't
		 */
		bool entry(const int data[], int size, bool newline);
	private:
		File log_file_;							/*< Log file being written to */
		char filename_[MAX_FILENAME_LENGTH];	/*< Filename of the log file */
		char header_[MAX_HEADER_LENGTH];		/*< Header to place at the top of the file */
		int chip_select_;						/*< Chip select pin for the SD card being used */
};

#endif