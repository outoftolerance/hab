#include "Log.h"

/*------------------------------Constructor Methods------------------------------*/
Log::Log(Stream& port, LOG_LEVELS log_level) :
  output_(port),
  log_level_(log_level)
{
    use_rtc_ = false;
}

Log::Log(Stream& port, RTC_DS3231* rtc, LOG_LEVELS log_level) :
  output_(port),
  clock_(rtc),
  log_level_(log_level)
{
    use_rtc_ = true;
}

/*------------------------------Public Methods------------------------------*/

void Log::init()
{
    static_cast<Serial_&>(output_).begin(57600);
}

void Log::event(LOG_LEVELS level, const char message[])
{
    if(level >= log_level_)
    {
        String preamble;

        if(clock_->isrunning() && use_rtc_)
        {
            DateTime now = clock_->now();
            preamble = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + " " + String(now.hour()) + " " + String(now.minute()) + " " + String(now.second());
            preamble += " | ";
            preamble += now.unixtime();
            preamble += " | ";
        }
        else
        {
            long now = millis();
            preamble = "0000/00/00 " + String(numberOfHours(now / 1000l)) + ":" + String(numberOfMinutes(now / 1000l)) + ":" + String(numberOfSeconds(now / 1000l)) + " | 0 | ";
        }

        switch(level)
        {
            case LOG_LEVELS::DEBUG:
                preamble += "DEBUG   | ";
                break;
            case LOG_LEVELS::INFO:
                preamble += "INFO    | ";
                break;
            case LOG_LEVELS::WARNING:
                preamble += "WARNING | ";
                break;
            case LOG_LEVELS::ERROR:
                preamble += "ERROR   | ";
                break;
            case LOG_LEVELS::FATAL:
                preamble += "FATAL   | ";
                break;
        }

        output_.print(preamble);
        output_.println(message);
    }
}

void Log::event(LOG_LEVELS level, const char message[], float data)
{
    if(level >= log_level_)
    {
        String preamble;

        if(clock_->isrunning() && use_rtc_)
        {
            DateTime now = clock_->now();
            preamble = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + " " + String(now.hour()) + " " + String(now.minute()) + " " + String(now.second());
            preamble += " | ";
            preamble += now.unixtime();
            preamble += " | ";
        }
        else
        {
            long now = millis();
            preamble = "0000/00/00 " + String(numberOfHours(now / 1000l)) + ":" + String(numberOfMinutes(now / 1000l)) + ":" + String(numberOfSeconds(now / 1000l)) + " | 0 | ";
        }

        switch(level)
        {
            case LOG_LEVELS::DEBUG:
                preamble = "DEBUG   | ";
                break;
            case LOG_LEVELS::INFO:
                preamble = "INFO    | ";
                break;
            case LOG_LEVELS::WARNING:
                preamble = "WARNING | ";
                break;
            case LOG_LEVELS::ERROR:
                preamble = "ERROR   | ";
                break;
            case LOG_LEVELS::FATAL:
                preamble = "FATAL   | ";
                break;
        }

        output_.print(preamble);
        output_.print(message);
        output_.print(": ");
        output_.println(data);
    }
}

void Log::event(LOG_LEVELS level, const char message[], int data)
{
    if(level >= log_level_)
    {
        String preamble;

        if(clock_->isrunning() && use_rtc_)
        {
            DateTime now = clock_->now();
            preamble = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + " " + String(now.hour()) + " " + String(now.minute()) + " " + String(now.second());
            preamble += " | ";
            preamble += now.unixtime();
            preamble += " | ";
        }
        else
        {
            long now = millis();
            preamble = "0000/00/00 " + String(numberOfHours(now / 1000l)) + ":" + String(numberOfMinutes(now / 1000l)) + ":" + String(numberOfSeconds(now / 1000l)) + " | 0 | ";
        }

        switch(level)
        {
            case LOG_LEVELS::DEBUG:
                preamble = "DEBUG   | ";
                break;
            case LOG_LEVELS::INFO:
                preamble = "INFO    | ";
                break;
            case LOG_LEVELS::WARNING:
                preamble = "WARNING | ";
                break;
            case LOG_LEVELS::ERROR:
                preamble = "ERROR   | ";
                break;
            case LOG_LEVELS::FATAL:
                preamble = "FATAL   | ";
                break;
        }

        output_.print(preamble);
        output_.print(message);
        output_.print(": ");
        output_.println(data);
    }
}
