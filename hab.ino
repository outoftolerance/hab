#include <Arduino.h>
#include "wiring_private.h" // For ATSAMD M0 pinPeripheral() function

#include <Timer.h>
#include <Telemetry.h>
#include <Log.h>
#include <DataLog.h>
#include <SimpleHDLC.h>
#include <SimpleMessageProtocol.h>
#include <MissionState.h>
#include <HardwareConfiguration.h>
#include <RTClib.h>

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

Stream& logging_output_stream = Serial;             /**< Logging output stream, this is of type Serial_ */
Stream& gps_input_stream = Serial1;                 /**< GPS device input stream, this is of type HardwareSerial */
Stream& radio_input_output_stream = Serial2;        /**< Radio input output stream, this is of type HardwareSerial */
Stream& cellular_input_output_stream = Serial3;     /**< Cellular input output stream, this is of type HardwareSerial */

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
void handleMessageTelemetryReport(hdlcMessage message);
void handleMessagePositionReport(hdlcMessage message);
void handleMessageCommandArm(hdlcMessage message);
void handleMessageCommandDisarm(hdlcMessage message);
void handleMessageCommandSetState(hdlcMessage message);
void handleMessageProtoAck(hdlcMessage message);
void handleMessageProtoNack(hdlcMessage message);

void sendTelemetryReport(TelemetryStruct& telemetry);
void logTelemetry(TelemetryStruct& telemetry);
void sendPositionReport(TelemetryStruct& telemetry);
void sendAck(MESSAGE_TYPES type);

SimpleHDLC radio(radio_input_output_stream, &handleMessageCallback);        /**< HDLC messaging object, linked to message callback */
SimpleHDLC cellular(cellular_input_output_stream, &handleMessageCallback);  /**< HDLC messaging object, linked to message callback */
RTC_DS3231 rtc;                                                             /**< Real Time Clock object */
Log logger(logging_output_stream, &rtc, LOG_LEVELS::DEBUG);                 /**< Log object */
DataLog telemetry_logger(SD_CHIP_SELECT);                                   /**< Data logging object for telemetry */
Telemetry telemetry(gps_input_stream, GPS_FIX_STATUS);                      /**< Telemetry object */
bool update_rtc_from_gps = false;                                           /**< If RTC lost power we need to update from GPS */

MissionState mission_state;         /**< Mission state state machine object */
Timer timer_telemetry_check;        /**< Timer sets interval between checking telemetry */
Timer timer_telemetry_report;       /**< Timer sets interval between reporting telemetry */
Timer timer_position_report;        /**< Timer sets interval between reporting position */
Timer timer_telemetry_log;          /**< Timer sets interval between logging telemetry */
Timer timer_execution_led;            /**< Timer sets intercal between run led blinks */

/**
 * @brief System setup function
 * @details Initialises all system componenets at start-up
 */
void setup() {
    //Sleep 5s so that debug can connect
    delay(5000);

    //Setup pin modes
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ARM_SWITCH, INPUT);
    pinMode(SILENCE_SWITCH, INPUT);
    pinMode(LED_EXTERNAL, OUTPUT);
    pinMode(GPS_FIX_STATUS, INPUT);
    pinMode(BUZZER_EXTERNAL, OUTPUT);
    
    //Start debug serial port
    logger.init();
    logger.event(LOG_LEVELS::INFO, "HAB systems starting...");

    //Start RTC
    logger.event(LOG_LEVELS::INFO, "Starting Real Time Clock...");
    if (!rtc.begin())
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Real Time Clock!");
        while(1);
    }

    //Set RTC if power was lost
    logger.event(LOG_LEVELS::INFO, "Checking real time clock status...");
    if (rtc.lostPower())
    {
        logger.event(LOG_LEVELS::WARNING, "RTC lost power, setting new date and time!");

        //Sets to when the sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

        update_rtc_from_gps = true;
    }

    //Start radio modem Serial port
    logger.event(LOG_LEVELS::INFO, "Starting radio modem...");
    static_cast<HardwareSerial&>(radio_input_output_stream).begin(57600);
    pinPeripheral(10, PIO_SERCOM);
    pinPeripheral(11, PIO_SERCOM);

    //Start cellular modem Serial port
    logger.event(LOG_LEVELS::INFO, "Starting cellular modem serial port...");
    static_cast<HardwareSerial&>(cellular_input_output_stream).begin(57600);
    pinPeripheral(3, PIO_SERCOM_ALT);
    pinPeripheral(4, PIO_SERCOM_ALT);

    //Initialise state machine
    logger.event(LOG_LEVELS::INFO, "Initialising Mission State subsystem...");
    if(!mission_state.set(MISSION_STATES::STAGING))
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Mission State subsystem!");
        while(1);
    }

    //Set initial program timers
    setTimers(mission_state.getFunction());

    //Initialise the telemetry system
    logger.event(LOG_LEVELS::INFO, "Initialising Telemetry subsystem...");
    if(!telemetry.init())
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Telemetry subsystem!");
        while(1);
    }

    String telemetry_log_name = "tlm.csv";
    String telemetry_log_header = "lat,lon,alt,alt_baro,roll,pitch,heading,course,temp,pres";

    //Start telemetry data logger
    logger.event(LOG_LEVELS::INFO, "Initialising Telemetry data logger...");
    if(!telemetry_logger.init(telemetry_log_name, telemetry_log_header))
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Telemetry data logger!");
        while(1);
    }
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
    timer_execution_led.setInterval(500);                   /**< Sets execution blinky LED interval to 500ms */

    //Start system timers
    timer_telemetry_check.start();
    timer_telemetry_report.start();
    timer_telemetry_log.start();
    timer_position_report.start();

    while(1)
    {
        //Get launch and silence switch states
        logger.event(LOG_LEVELS::DEBUG, "Getting updated status of switches.");
        launch_switch_state = digitalRead(ARM_SWITCH);
        silence_switch_state = digitalRead(SILENCE_SWITCH);

        //Telemetry Update
        if(timer_telemetry_check.check())
        {
            //Get latest telemetry
            logger.event(LOG_LEVELS::DEBUG, "Getting update from Telemetry subsystem.");

            if(!telemetry.get(current_telemetry))
            {
                logger.event(LOG_LEVELS::ERROR, "Failed to get update from Telemetry subsystem!");
            }
            else
            {
                logger.event(LOG_LEVELS::DEBUG, "Telemetry updated completed.");
                timer_telemetry_check.reset();
            }
        }

        //Set RTC if needed and gps fix is true
        if (update_rtc_from_gps && telemetry.getGpsFixStatus()) 
        {
            logger.event(LOG_LEVELS::INFO, "Setting RTC from GPS timestamp.");

            // This line sets the RTC with an explicit date & time from unix epoch
            rtc.adjust(DateTime(telemetry.getGpsDateTime()));

            logger.event(LOG_LEVELS::INFO, "RTC was adjusted from GPS timestamp.");
        }

        //Telemetry Report
        if(timer_telemetry_report.check())
        {
            logger.event(LOG_LEVELS::DEBUG, "Sending telemetry report message.");
            sendTelemetryReport(current_telemetry);

            timer_telemetry_report.reset();
        }

        //Telemetry Log
        if(timer_telemetry_log.check())
        {
            logger.event(LOG_LEVELS::DEBUG, "Logging telemetry to storage.");
            logTelemetry(current_telemetry);

            timer_telemetry_log.reset();
        }

        //Position Report
        if(timer_position_report.check())
        {
            logger.event(LOG_LEVELS::DEBUG, "Sending position report message.");
            sendPositionReport(current_telemetry);

            timer_position_report.reset();
        }

        //Buzzer Beeper
        if(current_mission_state_function.beeper_enabled)
        {
            digitalWrite(BUZZER_EXTERNAL, true);
        }
        else
        {
            digitalWrite(BUZZER_EXTERNAL, false);
        }

        //LED Blinker
        if(current_mission_state_function.led_enabled)
        {
            digitalWrite(LED_EXTERNAL, true);
        }
        else
        {
            digitalWrite(LED_EXTERNAL, false);
        }

        //Execution LED indicator blinkies
        if(timer_execution_led.check())
        {
            if(digitalRead(LED_BUILTIN) == HIGH)
            {
                digitalWrite(LED_BUILTIN, LOW);
            }
            else
            {
                digitalWrite(LED_BUILTIN, HIGH);
            }
        }

        //Update mission state
        logger.event(LOG_LEVELS::DEBUG, "Updating Mission State subsystem.");
        if(!mission_state.update(&current_telemetry, launch_switch_state, silence_switch_state))
        {
            logger.event(LOG_LEVELS::ERROR, "Failed to update Mission State subsystem!");
        }
        else
        {
            current_mission_state_function = mission_state.getFunction();
            logger.event(LOG_LEVELS::DEBUG, "Mission State subsystem update completed.");
        }

        //Update program timers based on state
        logger.event(LOG_LEVELS::DEBUG, "Setting system timers based on mission state.");
        setTimers(current_mission_state_function);

        //Print a bunch of debug information
        logger.event(LOG_LEVELS::DEBUG, "Current GPS Latitude   ", current_telemetry.latitude);
        logger.event(LOG_LEVELS::DEBUG, "Current GPS Longitude  ", current_telemetry.longitude);
        logger.event(LOG_LEVELS::DEBUG, "Current GPS Altitude   ", current_telemetry.altitude);
        logger.event(LOG_LEVELS::DEBUG, "Current GPS Course     ", current_telemetry.course);
        logger.event(LOG_LEVELS::DEBUG, "Current IMU Roll       ", current_telemetry.roll);
        logger.event(LOG_LEVELS::DEBUG, "Current IMU Pitch      ", current_telemetry.pitch);
        logger.event(LOG_LEVELS::DEBUG, "Current IMU Heading    ", current_telemetry.heading);
        logger.event(LOG_LEVELS::DEBUG, "Current IMU Altitude   ", current_telemetry.altitude_barometric);
        logger.event(LOG_LEVELS::DEBUG, "Current IMU Pressure   ", current_telemetry.pressure);
        logger.event(LOG_LEVELS::DEBUG, "Current IMU Temperature", current_telemetry.temperature);
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
    logger.event(LOG_LEVELS::INFO, "Received a message!");

    switch(message.command)
    {
        case MESSAGE_TYPES::MESSAGE_TYPE_REPORT_TELEMETRY:
            logger.event(LOG_LEVELS::INFO, "Received telemetry report.");
            handleMessageTelemetryReport(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_REPORT_POSITION:
            logger.event(LOG_LEVELS::INFO, "Received position report.");
            handleMessagePositionReport(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_ARM:
            logger.event(LOG_LEVELS::INFO, "Received takeoff command.");
            handleMessageCommandArm(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_DISARM:
            logger.event(LOG_LEVELS::INFO, "Received abort takeoff message.");
            handleMessageCommandDisarm(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_SET_STATE:
            logger.event(LOG_LEVELS::INFO, "Received set state command.");
            handleMessageCommandSetState(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_PROTO_ACK:
            logger.event(LOG_LEVELS::INFO, "Received acknowledgement.");
            handleMessageProtoAck(message);
            break;
    }
}

void handleMessageTelemetryReport(hdlcMessage message)
{
    logger.event(LOG_LEVELS::WARNING, "Ignoring telemetry report message.");
}

void handleMessagePositionReport(hdlcMessage message)
{
    logger.event(LOG_LEVELS::WARNING, "Ignoring position report message.");
}

void handleMessageCommandArm(hdlcMessage message)
{

}

void handleMessageCommandDisarm(hdlcMessage message)
{

}

void handleMessageCommandSetState(hdlcMessage message)
{

}

void handleMessageProtoAck(hdlcMessage message)
{

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

void sendPositionReport(TelemetryStruct& telemetry)
{
    hdlcMessage message;
    message.command = MESSAGE_TYPES::MESSAGE_TYPE_REPORT_POSITION;
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

void sendAck(MESSAGE_TYPES type)
{
    hdlcMessage message;
    message.command = MESSAGE_TYPES::MESSAGE_TYPE_PROTO_ACK;
    message.length = 1;
    message.payload[0] = type;

    radio.send(message);
    cellular.send(message);
}

void logTelemetry(TelemetryStruct& telemetry)
{
    int telemetry_size = 10;
    float temp_telemetry_array[telemetry_size];

    temp_telemetry_array[0] = telemetry.latitude;
    temp_telemetry_array[1] = telemetry.longitude;
    temp_telemetry_array[2] = telemetry.altitude;
    temp_telemetry_array[3] = telemetry.altitude_barometric;
    temp_telemetry_array[4] = telemetry.roll;
    temp_telemetry_array[5] = telemetry.pitch;
    temp_telemetry_array[6] = telemetry.heading;
    temp_telemetry_array[7] = telemetry.course;
    temp_telemetry_array[8] = telemetry.temperature;
    temp_telemetry_array[9] = telemetry.pressure;

    telemetry_logger.entry(temp_telemetry_array, telemetry_size, true);
}
