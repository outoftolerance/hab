#include <Timer.h>
#include <Telemetry.h>
#include <Log.h>
#include <SimpleHDLC.h>
#include <StateMachine.h>
#include <MessageDefinitions.h>

const bool debug = false; /**< Global debug flag, changes behaviour and outputs */

/**
 * @brief      Sets timers based on mission state
 */
void missionStateSetTimers();

/**
 * @brief      Callback function handles new messages from HDLC
 *
 * @param[in]  message  The message to be handled
 */
void handleMessageCallback(hdlcMessage message);

SimpleHDLC hdlc(&Serial, &handleMessageCallback); /**< HDLC messaging object */

Log logger(&Serial, debug); /**< Log object */

//Telemetry telemetry(&Serial1); /**< Telemetry object */

Timer timer_telemetry_check; /**< Timer sets interval between checking telemetry */
Timer timer_telemetry_report; /**< timer sets interval between reporting telemetry */
Timer timer_position_report; /**< Timer sets interval between reporting position */
Timer timer_telemetry_log; /**< timer sets interval between logging telemetry */

//Define some global variables
MissionState mission_state; /**<Enumerated variable tracks mission state */

/**
 * @brief System setup function
 * @details Initialises all system componenets at start-up
 */
void setup() {
    //Setup pin modes
    pinMode(LED_BUILTIN, OUTPUT);
    
    //Start debug serial port
    Serial.begin(57600);
    logger.info("HAB systems starting...");

    //Initialise state
    mission_state = STAGING;

    //Initialise the telemetry system
    logger.info("Initialising telemetry subsystem...");

/*
    if(!telemetry.init())
    {
        logger.fatal("Failed to initialise telemetry subsystem!");
        while(1);
    }
*/
    logger.info("Telemetry initialised successfully!");
}

/**
 * @brief Main program loop
 * @details Called after setup() function, loops inifiteley, everything happens here
 */
void loop() {
    //Check state of mission and set timers accordingly
    missionStateSetTimers();

    hdlcMessage test_message;

    test_message.command = MESSAGE_TYPE_REPORT_TELEMETRY;
    test_message.length = 1;
    test_message.payload[0] = (uint8_t)8;

    hdlc.send(&test_message);

    delay(5000);
}

void missionStateSetTimers()
{
    switch(mission_state)
    {
        case STAGING:
            timer_telemetry_check.setInterval(STAGING_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(STAGING_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(STAGING_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(STAGING_REPORT_POSITION_INTERVAL);
        case TAKEOFF:
            timer_telemetry_check.setInterval(TAKEOFF_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(TAKEOFF_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(TAKEOFF_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(TAKEOFF_REPORT_POSITION_INTERVAL);
        case ASCENDING:
            timer_telemetry_check.setInterval(ASCENDING_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(ASCENDING_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(ASCENDING_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(ASCENDING_REPORT_POSITION_INTERVAL);
        case DESCENDING:
            timer_telemetry_check.setInterval(DESCENDING_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(DESCENDING_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(DESCENDING_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(DESCENDING_REPORT_POSITION_INTERVAL);
        case LANDING:
            timer_telemetry_check.setInterval(LANDING_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(LANDING_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(LANDING_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(LANDING_REPORT_POSITION_INTERVAL);
        case RECOVERY:
            timer_telemetry_check.setInterval(RECOVERY_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(RECOVERY_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(RECOVERY_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(RECOVERY_REPORT_POSITION_INTERVAL);
        case RECOVERED:
            timer_telemetry_check.setInterval(RECOVERED_CHECK_TELEMETRY_INTERVAL);
            timer_telemetry_report.setInterval(RECOVERED_REPORT_TELEMETRY_INTERVAL);
            timer_telemetry_log.setInterval(RECOVERED_LOG_TELEMETRY_INTERVAL);
            timer_position_report.setInterval(RECOVERED_REPORT_POSITION_INTERVAL);
    }
}

void handleMessageCallback(hdlcMessage message)
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}
