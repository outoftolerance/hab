#include "Timer.h"

/*------------------------------Constructor Methods------------------------------*/

Timer::Timer()
{
	interval_ = DEFAULT_INTERVAL;
	is_set_ = false;
}

Timer::Timer(unsigned long interval)
{
	interval_ = interval;
	is_set_ = true;
}

/*------------------------------ Public Methods------------------------------*/

bool Timer::setInterval(unsigned long interval)
{
	interval_ = interval;
	is_set_ = true;
	return true;
}

unsigned long Timer::getInterval()
{
	return interval_;
}

bool Timer::isSet()
{
	return is_set_;
}

bool Timer::isStarted()
{
	return is_running_;
}

bool Timer::start()
{
	if(!is_set_ || is_running_)
	{
		return false;
	}

	last_reset_ = millis();
	is_running_ = true;

	return true;
}

bool Timer::stop()
{
	if(!is_running_ || !is_set_)
	{
		return false;
	}

	is_running_ = false;

	return true;
}

bool Timer::check()
{
	if(!is_running_)
	{
		return false;
	}

	if(millis() - last_reset_ > interval_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Timer::reset()
{
	if(!check())
	{
		return false;
	}

	last_reset_ = millis();
	return true;
}

bool Timer::forceReset()
{
	last_reset_ = millis();
	return true;	
}