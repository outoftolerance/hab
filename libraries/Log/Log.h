#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include <RTClib.h>

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)

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
        * @param      log_level The level at which to log above (inclusive)
        */
        Log(Stream& port, LOG_LEVELS log_level);

        /**
        * @brief      Logger actual constructor
        * @param      port      Stream pointer to output stream used
        * @param      rtc       Pointer to a real time clock to use for timestamping
        * @param      log_level The level at which to log above (inclusive)
        */
        Log(Stream& port, RTC_DS3231* rtc, LOG_LEVELS log_level);

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
        RTC_DS3231* clock_;         /**< Pointer to RTC used for timing, not a reference because clock is optional */
        int log_level_;             /**< Sets logging level */
        bool use_rtc_;              /**< Sets whether RTC is being used or not */
};

#endif
