#include "Log.h"

/*------------------------------Constructor Methods------------------------------*/
Log::Log(Stream& terminal, bool debug) :
  output_(terminal),
  debug_(debug)
{

}

/*------------------------------Public Methods------------------------------*/

void Log::info(const char message[])
{
  output_.print("INFO: ");
  output_.println(message);
}

void Log::debug(const char message[])
{
  if(debug_)
  {
    output_.print("DEBUG: ");
    output_.println(message);
  }
}

void Log::error(const char message[])
{
  output_.print("ERROR: ");
  output_.println(message);
}

void Log::fatal(const char message[])
{
  output_.print("FATAL: ");
  output_.println(message);
}
