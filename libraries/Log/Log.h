#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include <RTClib.h>

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
        * @param      port      Stream pointer to output stream used
        * @param[in]  debug     Boolean to activate debug mode
        */
        Log(Stream& port, RTC_DS3231& rtc, LOG_LEVELS log_level);

        void init();

        /**
        * @brief      Infomation log level
        * @param      message  The message to be logged
        */
        void event(LOG_LEVELS level, const char message[]);

        /**
        * @brief      Infomation log level
        * @param      message  The message to be logged
        * @param      data  A number to be logged with the string
        */
        void event(LOG_LEVELS level, const char message[], float data);

        /**
        * @brief      Infomation log level
        * @param      message  The message to be logged
        * @param      data  A number to be logged with the string
        */
        void event(LOG_LEVELS level, const char message[], int data);

    private:
        Stream& output_;            /**< Reference to stream used for output */
        RTC_DS3231& clock_;         /**< Reference to RTC used for timing */
        int log_level_;             /**< Debug mode flag */
};

#endif
