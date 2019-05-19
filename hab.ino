#include <Arduino.h>
#include "wiring_private.h" // For ATSAMD M0 pinPeripheral() function

#include <Timer.h>
#include <Telemetry.h>
#include <Log.h>
#include <SimpleHDLC.h>
#include <MissionState.h>
#include <HardwareConfiguration.h>

#define DEBUG false                                                 /**< Global debug flag, changes behaviour and outputs */

/*
 * Creating some new Serial ports using M0 SERCOM for peripherals
 *
 * Notes for Adafruit Feather M0 pre-defined Serial ports:
 *     - Serial goes to USB port interface (PA24, PA25)
 *     - Serial1 is broken out on the board and uses pins 1/PA10 (TX), 0/PA11 (RX)
 *     - Serial5 is on pins 30/PB22 (TX), 31/PB23 (RX) but not exposed on the board
 * 
 * We are adding the following:
 *     - Serial2 on pins 10/PA18 (TX), 11/PA16 (RX)
 *     - Serial3 on pins 4/PA08 (TX), 3/PA09 (RX)
 */
Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);    /**< Creating a second serial port using SERCOM1 */
Uart Serial3 (&sercom2, 3, 4, SERCOM_RX_PAD_1, UART_TX_PAD_0);      /**< Creating a third serial port using SERCOM2 */

/*
 * @brief Handler function for SERCOM1 (serial port 2)
 */
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

/*
 * @brief Handler function for SERCOM2 (serial port 3)
 */
void SERCOM2_Handler()
{
  Serial3.IrqHandler();
}

Stream& logging_output_stream = Serial;             /**< Logging output stream */
Stream& gps_input_stream = Serial2;                 /**< GPS device input stream */
Stream& radio_input_output_stream = Serial3;        /**< Radio input output stream */
Stream& cellular_input_output_stream = Serial5;     /**< Cellular output stream (Serial5 pre-defined for Feather M0 on SERCOM5) */

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

SimpleHDLC radio(radio_input_output_stream, &handleMessageCallback);        /**< HDLC messaging object, linked to message callback */
SimpleHDLC cellular(cellular_input_output_stream, &handleMessageCallback);  /**< HDLC messaging object, linked to message callback */
Log logger(logging_output_stream, DEBUG);                                   /**< Log object */
Telemetry telemetry(gps_input_stream);                                      /**< Telemetry object */

MissionState mission_state;         /**< Mission state state machine object */
Timer timer_telemetry_check;        /**< Timer sets interval between checking telemetry */
Timer timer_telemetry_report;       /**< timer sets interval between reporting telemetry */
Timer timer_position_report;        /**< Timer sets interval between reporting position */
Timer timer_telemetry_log;          /**< timer sets interval between logging telemetry */

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
    static_cast<HardwareSerial&>(logging_output_stream).begin(57600);
    logger.info("HAB systems starting...");

    //Start GPS Serial port
    logger.info("Starting GPS serial port...");
    static_cast<HardwareSerial&>(gps_input_stream).begin(57600);
    pinPeripheral(10, PIO_SERCOM);
    pinPeripheral(11, PIO_SERCOM);

    //Start radio modem Serial port
    logger.info("Starting radio modem serial port...");
    static_cast<HardwareSerial&>(radio_input_output_stream).begin(57600);
    pinPeripheral(3, PIO_SERCOM_ALT);
    pinPeripheral(4, PIO_SERCOM_ALT);

    //Start cellular modem Serial port
    logger.info("Starting cellular modem serial port...");
    static_cast<HardwareSerial&>(cellular_input_output_stream).begin(57600);

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
            if(!telemetry.get(current_telemetry))
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
            sendPositionReport(current_telemetry);

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

            radio.send(test_message);
            cellular.send(test_message);
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

    for (i = 0; i < message.length; i++)
    {
        message.payload[i] = *temp_pointer++;
    }

    radio.send(message);
    cellular.send(message);
}

void logTelemetry(TelemetryStruct& telemetry)
{

}

void sendPositionReport(TelemetryStruct& telemetry)
{
    hdlcMessage message;
    message.command = MESSAGE_TYPE_REPORT_POSITION;
    message.length = 3 * sizeof(float);

    int i = 0;
    const uint8_t* temp_pointer = (const uint8_t*)&telemetry;

    for (i = 0; i < message.length; i++)
    {
        message.payload[i] = *temp_pointer++;
    }

    radio.send(message);
    cellular.send(message);
}
