#ifndef MISSION_STATE_H
#define MISSION_STATE_H

#include <Telemetry.h>
#include <Timer.h>

/**
 * @brief      Enumerator defines different states in the HAB mission
 */
enum MISSION_STATES 
{
    STAGING,
    TAKEOFF,
    ASCENDING,
    DESCENDING,
    LANDING,
    RECOVERY,
    RECOVERED
};

/**
 * @brief A type to report mission state functions
 */
typedef struct
{
    unsigned long telemetry_check_interval;
    unsigned long telemetry_report_interval;
    unsigned long telemetry_log_interval;
    unsigned long position_report_interval;
    bool beeper_enabled;
    bool led_enabled;
} MissionStateFunction;

/**
 * Define the behaviour of the system under different states
 */
#define STAGING_TELEMETRY_CHECK_INTERVAL 1000
#define STAGING_TELEMETRY_REPORT_INTERVAL 5000
#define STAGING_TELEMETRY_LOG_INTERVAL 5000
#define STAGING_POSITION_REPORT_INTERVAL 30000
#define STAGING_BEEPER_ENABLED false
#define STAGING_LED_ENABLED true

#define TAKEOFF_TELEMETRY_CHECK_INTERVAL 1000
#define TAKEOFF_TELEMETRY_REPORT_INTERVAL 15000
#define TAKEOFF_TELEMETRY_LOG_INTERVAL 5000
#define TAKEOFF_POSITION_REPORT_INTERVAL 15000
#define TAKEOFF_BEEPER_ENABLED true
#define TAKEOFF_LED_ENABLED true

#define ASCENDING_TELEMETRY_CHECK_INTERVAL 1000
#define ASCENDING_TELEMETRY_REPORT_INTERVAL 60000
#define ASCENDING_TELEMETRY_LOG_INTERVAL 5000
#define ASCENDING_POSITION_REPORT_INTERVAL 30000
#define ASCENDING_BEEPER_ENABLED false
#define ASCENDING_LED_ENABLED false

#define DESCENDING_TELEMETRY_CHECK_INTERVAL 1000
#define DESCENDING_TELEMETRY_REPORT_INTERVAL 60000
#define DESCENDING_TELEMETRY_LOG_INTERVAL 5000
#define DESCENDING_POSITION_REPORT_INTERVAL 30000
#define DESCENDING_BEEPER_ENABLED false
#define DESCENDING_LED_ENABLED false

#define LANDING_TELEMETRY_CHECK_INTERVAL 1000
#define LANDING_TELEMETRY_REPORT_INTERVAL 60000
#define LANDING_TELEMETRY_LOG_INTERVAL 5000
#define LANDING_POSITION_REPORT_INTERVAL 5000
#define LANDING_BEEPER_ENABLED true
#define LANDING_LED_ENABLED true

#define RECOVERY_TELEMETRY_CHECK_INTERVAL 1000
#define RECOVERY_TELEMETRY_REPORT_INTERVAL 300000
#define RECOVERY_TELEMETRY_LOG_INTERVAL 5000
#define RECOVERY_POSITION_REPORT_INTERVAL 300000
#define RECOVERY_BEEPER_ENABLED true
#define RECOVERY_LED_ENABLED true

#define RECOVERED_TELEMETRY_CHECK_INTERVAL 1000
#define RECOVERED_TELEMETRY_REPORT_INTERVAL 900000
#define RECOVERED_TELEMETRY_LOG_INTERVAL 5000
#define RECOVERED_POSITION_REPORT_INTERVAL 900000
#define RECOVERED_BEEPER_ENABLED false
#define RECOVERED_LED_ENABLED false

#define TERMINAL_ALTITUDE 500
#define SILENCE_DETECTION_TIMEOUT_INTERVAL 5000
#define DESCENT_DETECTION_TIMEOUT_INTERVAL 5000
#define LANDING_DETECTION_TIMEOUT_INTERVAL 5000
#define LANDED_ALTITUDE_DEADZONE 5
#define RECOVERED_INNACTIVITY_TIMEOUT_INTERVAL 300000

/**
 * @brief Mission state class
 * @note Provides state machine tracking for mission and associated functions
 */
class MissionState
{
    public:
        MissionState();
        ~MissionState();

        /**
         * @brief Update the mission state with latest information
         * @return bool True if successfully updated, false if error reported
         */
        bool update(TelemetryStruct* telemetry, bool launch_switch, bool silence_switch);

        /**
         * @brief Allows setting of the mission state to a particular state
         * @return bool True if success, false if error reported
         */
        bool set(int state);

        /**
         * @brief Returns the current state of the mission
         * @return MissionStates enumerated mission state
         */
        int get();

        /**
         * @brief Returns the current state of the functions related to mission states
         * @return MissionStateFunction type with values based on current mission state
         */
        MissionStateFunction getFunction();

    private:
        int current_mission_state_;                             /**< Current enumerated state of the mission */
        MissionStateFunction current_mission_state_function_;   /**< Functionality of the current mission state */
        Timer recovered_timeout_;                               /**< Timer for recovered mode time-out */
        Timer silence_timeout_;                                 /**< Timer for silence button time-out */
        Timer descent_timeout_;                                 /**< Timer for checking descent */
        Timer landing_timeout_;                                 /**< Timer for checking landing */
        float previous_altitude_;                               /**< Stores previous loop's altitude */
};

#endif
