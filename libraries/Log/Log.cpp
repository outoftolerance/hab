#include "Log.h"

/*------------------------------Constructor Methods------------------------------*/
Log::Log(Stream& port, RTC_DS3231& rtc, LOG_LEVELS log_level) :
  output_(port),
  clock_(rtc),
  log_level_(log_level)
{

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

        if(clock_.isrunning())
        {
            DateTime now = clock_.now();
            preamble = now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + " " + now.minute() + " " + now.second();
            preamble += " | "
            preamble += now.now();
            preamble += " | ";
        }
        else
        {
            preamble = "0000/00/00 00:00:00 | 0 | ";
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

        if(clock_.isrunning())
        {
            DateTime now = clock_.now();
            preamble = now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + " " + now.minute() + " " + now.second();
            preamble += " | "
            preamble += now.now();
            preamble += " | ";
        }
        else
        {
            preamble = "0000/00/00 00:00:00 | 0 | ";
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

        if(clock_.isrunning())
        {
            DateTime now = clock_.now();
            preamble = now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + " " + now.minute() + " " + now.second();
            preamble += " | "
            preamble += now.now();
            preamble += " | ";
        }
        else
        {
            preamble = "0000/00/00 00:00:00 | 0 | ";
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
