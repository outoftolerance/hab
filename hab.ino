#include <Timer.h>
#include <Telemetry.h>
#include <Log.h>
#include <SimpleHDLC.h>
#include <MissionState.h>
#include <MessageDefinitions.h>
#include <HardwareConfiguration.h>

const bool debug = false;                           /**< Global debug flag, changes behaviour and outputs */
const Stream* gps_input_stream = &Serial;           /**< GPS device input stream */
const Stream* logging_output_stream = &Serial;      /**< Logging output stream */
const Stream* messaging_output_stream = &Serial;    /**< Messaging output stream */

bool launch_switch_state = false;                   /**< Current launch switch state */
bool silence_switch_state = false;                  /**< Current silsnce switch state */

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

SimpleHDLC hdlc(messaging_output_stream, &handleMessageCallback);   /**< HDLC messaging object */
Log logger(logging_output_stream, debug);                           /**< Log object */
Telemetry telemetry(gps_input_stream);                              /**< Telemetry object */

MissionState mission_state;                                         /**< Mission state state machine object */
Timer timer_telemetry_check;                                        /**< Timer sets interval between checking telemetry */
Timer timer_telemetry_report;                                       /**< timer sets interval between reporting telemetry */
Timer timer_position_report;                                        /**< Timer sets interval between reporting position */
Timer timer_telemetry_log;                                          /**< timer sets interval between logging telemetry */

//Define some global variables
TelemetryStruct current_telemetry;

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
    logger.info("Initialising state machine subsystem...");
    mission_state.set(MissionStates::STAGING);
    setTimers(mission_state.getFunction());
    logger.info("State machine initialised successfully!");

    //Initialise the telemetry system
    logger.info("Initialising telemetry subsystem...");
    if(!telemetry.init())
    {
        logger.fatal("Failed to initialise telemetry subsystem!");
        while(1);
    }
    logger.info("Telemetry initialised successfully!");
}

/**
 * @brief Main program loop
 * @details Called after setup() function, loops inifiteley, everything happens here
 */
void loop() {
    while(1)
    {
        //Get latest telemetry
        logger.info("Getting updated telemetry from sensors.");
        telemetry.get(&current_telemetry);

        //Get launch and silsnce switch states
        logger.info("Getting updated status of switches.");
        launch_switch_state = digitalRead(LAUNCH_SWITCH);
        silence_switch_state = digitalRead(SILENCE_SWITCH);

        //Update mission state based on telemetry
        logger.info("Updating mission state machine.");
        mission_state.update(&current_telemetry, launch_switch_state, silence_switch_state);

        //Check state of mission and set timers accordingly
        logger.info("Setting system timers based on mission state.");
        setTimers(mission_state.getFunction());

        //Send a test message
        logger.info("Sending testing message.");
        hdlcMessage test_message;
        test_message.command = MESSAGE_TYPE_REPORT_TELEMETRY;
        test_message.length = 1;
        test_message.payload[0] = (uint8_t)8;
        hdlc.send(&test_message);

        //Delay loop for testing, get rid of this in the for reals code
        logger.info("Delaying loop by 5s...");
        delay(5000);
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
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}
