#include "Log.h"

/*------------------------------Constructor Methods------------------------------*/

Log::Log()
{

}

Log::Log(Stream* terminal, bool debug)
{
  output_ = terminal;
  debug_ = debug;
}

/*------------------------------Public Methods------------------------------*/

void Log::info(char message[])
{
  output_->print("INFO: ");
  output_->println(message);
}

void Log::debug(char message[])
{
  if(debug_)
  {
    output_->print("DEBUG: ");
    output_->println(message);
  }
}

void Log::error(char message[])
{
  output_->print("ERROR: ");
  output_->println(message);
}

void Log::fatal(char message[])
{
  output_->print("FATAL: ");
  output_->println(message);
}
