#include <Timer.h>
#include <Telemetry.h>
#include <Log.h>
#include <SimpleHDLC.h>
#include <MissionState.h>
#include <HardwareConfiguration.h>

#define DEBUG false                          /**< Global debug flag, changes behaviour and outputs */

Stream& gps_input_stream = Serial;           /**< GPS device input stream */
Stream& logging_output_stream = Serial;      /**< Logging output stream */
Stream& messaging_output_stream = Serial;    /**< Messaging output stream */

enum MESSAGE_TYPES {
    MESSAGE_TYPE_REPORT_TELEMETRY,
    MESSAGE_TYPE_REPORT_POSITION,

    MESSAGE_TYPE_COMMAND_TAKEOFF,
    MESSAGE_TYPE_COMMAND_ABORT_TAKEOFF,
    MESSAGE_TYPE_COMMAND_SET_STATE,

    MESSAGE_TYPE_PROTO_ACK,
    MESSAGE_TYPE_PROTO_NACK
};

/**
 * @brief      Sets timers based on mission state
 */
void setTimers(MissionStateFunction function);

/**
 * @brief      Callback function handles new messages from HDLC
 *
 * @param[in]  message  The message to be handled
 */
void handleMessageCallback(hdlcMessage message);

SimpleHDLC hdlc(messaging_output_stream, &handleMessageCallback);   /**< HDLC messaging object, linked to message callback */
Log logger(logging_output_stream, DEBUG);                           /**< Log object */
Telemetry telemetry(gps_input_stream);                              /**< Telemetry object */

MissionState mission_state;                                         /**< Mission state state machine object */
Timer timer_telemetry_check;                                        /**< Timer sets interval between checking telemetry */
Timer timer_telemetry_report;                                       /**< timer sets interval between reporting telemetry */
Timer timer_position_report;                                        /**< Timer sets interval between reporting position */
Timer timer_telemetry_log;                                          /**< timer sets interval between logging telemetry */

/**
 * @brief System setup function
 * @details Initialises all system componenets at start-up
 */
void setup() {
    //Setup pin modes
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LAUNCH_SWITCH, INPUT);
    pinMode(SILENCE_SWITCH, INPUT);
    
    //Start debug serial port
    Serial.begin(57600);
    logger.info("HAB systems starting...");

    //Initialise state machine
    logger.info("Initialising Mission State subsystem...");
    if(!mission_state.set(MISSION_STATES::STAGING))
    {
        logger.fatal("Failed to initialise Mission State subsystem!");
        while(1);
    }
    logger.info("Mission State initialised successfully!");

    //Initialise the telemetry system
    logger.info("Initialising Telemetry subsystem...");
    if(!telemetry.init())
    {
        logger.fatal("Failed to initialise Telemetry subsystem!");
        while(1);
    }
    logger.info("Telemetry initialised successfully!");

    //Set initial program timers
    setTimers(mission_state.getFunction());
}

/**
 * @brief Main program loop
 * @details Called after setup() function, loops inifiteley, everything happens here
 */
void loop() {
    bool launch_switch_state = false;                       /**< Current launch switch state */
    bool silence_switch_state = false;                      /**< Current silsnce switch state */
    MissionStateFunction current_mission_state_function;    /**< Current mission state function */
    TelemetryStruct current_telemetry;                      /**< Current telemetry */

    while(1)
    {
        /*---Execute program elements based on timers---*/

        //Get launch and silsnce switch states
        logger.info("Getting updated status of switches.");
        launch_switch_state = digitalRead(LAUNCH_SWITCH);
        silence_switch_state = digitalRead(SILENCE_SWITCH);

        //Telemetry Update
        if(timer_telemetry_check.check())
        {
            //Get latest telemetry
            logger.info("Getting update from Telemetry subsystem.");
            if(!telemetry.get(&current_telemetry))
            {
                logger.error("Failed to get update from Telemetry subsystem!");
            }
            else
            {
                logger.info("Telemetry updated completed.");
                timer_telemetry_check.reset();
            }
        }

        //Telemetry Report
        if(timer_telemetry_report.check())
        {
            sendTelemetryReport(current_telemetry);

            timer_telemetry_report.reset();
        }

        //Telemetry Log
        if(timer_telemetry_log.check())
        {
            //stuff

            timer_telemetry_log.reset();
        }

        //Position Report
        if(timer_position_report.check())
        {
            //stuff

            timer_position_report.reset();
        }

        //Buzzer Beeper
        if(current_mission_state_function.beeper_enabled)
        {
            //stuff
        }

        //LED Blinker
        if(current_mission_state_function.led_enabled)
        {
            //stuff
        }

        /*---Update program controls--*/

        //Update mission state
        logger.info("Updating Mission State subsystem.");
        if(!mission_state.update(&current_telemetry, launch_switch_state, silence_switch_state))
        {
            logger.error("Failed to update Mission State subsystem!");
        }
        else
        {
            current_mission_state_function = mission_state.getFunction();
            logger.info("Mission State subsystem update completed.");
        }

        //Update program timers based on state
        logger.info("Setting system timers based on mission state.");
        setTimers(current_mission_state_function);

        /*---Misc---*/

        //Set a test message if in debug mode
        if(DEBUG)
        {
            //Send a test message
            logger.info("Sending testing message.");
            hdlcMessage test_message;
            test_message.command = MESSAGE_TYPE_REPORT_TELEMETRY;
            test_message.length = 1;
            test_message.payload[0] = (uint8_t)8;
            hdlc.send(test_message);
        }
    }
}

void setTimers(MissionStateFunction function)
{    
    timer_telemetry_check.setInterval(function.telemetry_check_interval);
    timer_telemetry_report.setInterval(function.telemetry_report_interval);
    timer_telemetry_log.setInterval(function.telemetry_log_interval);
    timer_position_report.setInterval(function.position_report_interval);
}

void handleMessageCallback(hdlcMessage message)
{
    switch(message.command)
    {
        case MESSAGE_TYPES::MESSAGE_TYPE_REPORT_TELEMETRY:
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_REPORT_POSITION:
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_TAKEOFF:
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_ABORT_TAKEOFF:
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_SET_STATE:
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_PROTO_ACK:
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_PROTO_NACK:
            break;
    }
}

void sendTelemetryReport(TelemetryStruct& telemetry)
{
    hdlcMessage message;
    message.command = MESSAGE_TYPES::MESSAGE_TYPE_REPORT_TELEMETRY;
    message.length = sizeof(telemetry);

    int i = 0;
    const uint8_t* temp_pointer = (const uint8_t*)&telemetry;

    for (i = 0; i < sizeof(telemetry); i++)
    {
        message.payload[i] = *temp_pointer++;
    }
    
    hdlc.send(message);
}

void logTelemetry(TelemetryStruct* telemetry)
{
    
}

void sendPositionReport(TelemetryStruct* telemetry)
{
    hdlcMessage message;
    message.command = MESSAGE_TYPE_REPORT_POSITION;
    message.length = 0;
    hdlc.send(message);
}
