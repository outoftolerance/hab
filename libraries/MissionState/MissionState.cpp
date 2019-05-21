#include "MissionState.h"

MissionState::MissionState()
{
	descent_timeout_.setInterval(DESCENT_DETECTION_TIMEOUT_INTERVAL);
	recovered_timeout_.setInterval(RECOVERED_INNACTIVITY_TIMEOUT_INTERVAL);
	silence_timeout_.setInterval(SILENCE_DETECTION_TIMEOUT_INTERVAL);
	landing_timeout_.setInterval(LANDING_DETECTION_TIMEOUT_INTERVAL);
}

MissionState::~MissionState()
{
	descent_timeout_.stop();
	descent_timeout_.forceReset();

	recovered_timeout_.stop();
	recovered_timeout_.forceReset();

	silence_timeout_.stop();
	silence_timeout_.forceReset();

	landing_timeout_.stop();
	landing_timeout_.forceReset();
}

bool MissionState::update(TelemetryStruct* telemetry, bool launch_switch, bool silence_switch)
{
	switch(current_mission_state_)
	{
		case MISSION_STATES::STAGING:
			if(launch_switch == true)
			{
				current_mission_state_ = MISSION_STATES::TAKEOFF;
			}
		case MISSION_STATES::TAKEOFF:
			if(launch_switch == false)
			{
				current_mission_state_ = MISSION_STATES::STAGING;
			}
			else if(telemetry->altitude >= TERMINAL_ALTITUDE)
			{
				current_mission_state_ = MISSION_STATES::ASCENDING;
			}
		case MISSION_STATES::ASCENDING:
			if(telemetry->altitude < previous_altitude_)
			{
				if(descent_timeout_.isStarted())
				{
					if(descent_timeout_.check())
					{
						current_mission_state_ = MISSION_STATES::DESCENDING;
						descent_timeout_.stop();
						descent_timeout_.reset();
					}
				}
				else
				{
					descent_timeout_.start();
				}
			}
			else
			{
				if(descent_timeout_.isStarted())
				{
					descent_timeout_.stop();
					descent_timeout_.forceReset();
				}
			}

			previous_altitude_ = telemetry->altitude;
		case MISSION_STATES::DESCENDING:
			if(telemetry->altitude <= TERMINAL_ALTITUDE)
			{
				current_mission_state_ = MISSION_STATES::LANDING;
			}
		case MISSION_STATES::LANDING:
			if(telemetry->altitude <= previous_altitude_ + LANDED_ALTITUDE_DEADZONE && telemetry->altitude >= previous_altitude_ - LANDED_ALTITUDE_DEADZONE)
			{
				if(landing_timeout_.isStarted())
				{
					if(landing_timeout_.check())
					{
						current_mission_state_ = MISSION_STATES::RECOVERY;
						landing_timeout_.stop();
						landing_timeout_.reset();
					}
				}
				else
				{
					landing_timeout_.start();
				}
			}
			else
			{
				if(landing_timeout_.isStarted())
				{
					landing_timeout_.stop();
					landing_timeout_.forceReset();
				}
			}

			previous_altitude_ = telemetry->altitude;
		case MISSION_STATES::RECOVERY:
			if(silence_switch)
			{
				if(silence_timeout_.isStarted())
				{
					if(silence_timeout_.check())
					{
						current_mission_state_ = MISSION_STATES::RECOVERED;
						silence_timeout_.stop();
						silence_timeout_.reset();
					}
				}
				else
				{
					silence_timeout_.start();
				}
			}
			else
			{
				if(silence_timeout_.isStarted())
				{
					silence_timeout_.stop();
					silence_timeout_.forceReset();
				}
			}
		case MISSION_STATES::RECOVERED:
			if(recovered_timeout_.isStarted())
			{
				if(recovered_timeout_.check())
				{
					current_mission_state_ = MISSION_STATES::RECOVERY;
					recovered_timeout_.stop();
					recovered_timeout_.reset();
				}
			}
			else
			{
				recovered_timeout_.start();
			}
	}

	return true;
}

bool MissionState::set(int state)
{
	current_mission_state_ = state;

	return true;
}

int MissionState::get()
{
	return current_mission_state_;
}

MissionStateFunction MissionState::getFunction()
{
	MissionStateFunction function;

	switch(current_mission_state_)
	{
		case MISSION_STATES::STAGING:
			function.telemetry_check_interval = STAGING_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = STAGING_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = STAGING_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = STAGING_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = STAGING_BEEPER_ENABLED;
			function.led_enabled = STAGING_LED_ENABLED;
		case MISSION_STATES::TAKEOFF:
			function.telemetry_check_interval = TAKEOFF_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = TAKEOFF_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = TAKEOFF_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = TAKEOFF_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = TAKEOFF_BEEPER_ENABLED;
			function.led_enabled = TAKEOFF_LED_ENABLED;
		case MISSION_STATES::ASCENDING:
			function.telemetry_check_interval = ASCENDING_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = ASCENDING_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = ASCENDING_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = ASCENDING_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = ASCENDING_BEEPER_ENABLED;
			function.led_enabled = ASCENDING_LED_ENABLED;
		case MISSION_STATES::DESCENDING:
			function.telemetry_check_interval = DESCENDING_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = DESCENDING_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = DESCENDING_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = DESCENDING_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = DESCENDING_BEEPER_ENABLED;
			function.led_enabled = DESCENDING_LED_ENABLED;
		case MISSION_STATES::LANDING:
			function.telemetry_check_interval = LANDING_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = LANDING_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = LANDING_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = LANDING_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = LANDING_BEEPER_ENABLED;
			function.led_enabled = LANDING_LED_ENABLED;
		case MISSION_STATES::RECOVERY:
			function.telemetry_check_interval = RECOVERY_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = RECOVERY_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = RECOVERY_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = RECOVERY_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = RECOVERY_BEEPER_ENABLED;
			function.led_enabled = RECOVERY_LED_ENABLED;
		case MISSION_STATES::RECOVERED:
			function.telemetry_check_interval = RECOVERY_TELEMETRY_CHECK_INTERVAL;
			function.telemetry_report_interval = RECOVERY_TELEMETRY_REPORT_INTERVAL;
			function.telemetry_log_interval = RECOVERY_TELEMETRY_LOG_INTERVAL;
			function.position_report_interval = RECOVERY_POSITION_REPORT_INTERVAL;
			function.beeper_enabled = RECOVERY_BEEPER_ENABLED;
			function.led_enabled = RECOVERY_LED_ENABLED;
	}

	return function;
}