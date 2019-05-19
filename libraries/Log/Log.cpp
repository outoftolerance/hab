#include "Log.h"

/*------------------------------Constructor Methods------------------------------*/
Log::Log(Stream& terminal, LOG_LEVELS log_level) :
  output_(terminal),
  log_level_(log_level)
{

}

/*------------------------------Public Methods------------------------------*/

void Log::event(LOG_LEVELS level, const char message[])
{
    if(level >= log_level_)
    {
        String preamble;

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

        output_.println(preamble);
        output_.println(message);
    }
}
