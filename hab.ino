#include <Arduino.h>
#include <Servo.h>
#include "wiring_private.h" // For ATSAMD M0 pinPeripheral() function

#include <SimpleUtils.h>
#include <Timer.h>
#include <Telemetry.h>
#include <Log.h>
#include <DataLog.h>
#include <SimpleHDLC.h>
#include <SimpleMessageProtocol.h>
#include <MissionState.h>
#include <HardwareConfiguration.h>
#include <RTClib.h>
#include <Adafruit_PWMServoDriver.h>
#include <SimpleServo.h>
#include <SimpleMusic.h>
#include <Adafruit_SleepyDog.h>

/*
 * Creating some new Serial ports using M0 SERCOM for peripherals
 *
 * Notes for Adafruit Feather M0 pre-defined Serial ports:
 *     - SerialUSB goes to USB port interface (PA24, PA25)
 *     - Serial1 is broken out on the board and uses pins 1/PA10 (TX), 0/PA11 (RX)
 *     - Serial is on pins 30/PB22 (TX), 31/PB23 (RX) but not exposed on the board
 *     - To get this config though we comment out some lines in the variants file
 * 
 * We are adding the following:
 *     - Serial2 on pins 10/PA18 (TX), 11/PA16 (RX)
 *     - Serial3 on pins 4/PA08 (TX), 3/PA09 (RX) but not exposed on the board
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
Stream& radio_input_output_stream = Serial3;        /**< Radio input output stream, this is of type HardwareSerial */
Stream& cellular_input_output_stream = Serial2;     /**< Cellular input output stream, this is of type HardwareSerial */
Stream& aprs_output_stream = Serial5;               /**< APRS output data stream, this is of type Serial_ */

SimpleHDLC radio(radio_input_output_stream, &handleMessageCallback);                            /**< HDLC messaging object, linked to message callback */
SimpleHDLC cellular(cellular_input_output_stream, &handleMessageCallback);                      /**< HDLC messaging object, linked to message callback */
RTC_DS3231 rtc;                                                                                 /**< Real Time Clock object */
Log logger(logging_output_stream, &rtc, LOG_LEVELS::INFO);                                      /**< Log object */
DataLog telemetry_logger(SD_CHIP_SELECT, &rtc);                                                 /**< Data logging object for telemetry */
Telemetry telemetry(&logger, &gps_input_stream);                                                /**< Telemetry object */
bool update_rtc_from_gps = false;                                                               /**< If RTC lost power we need to update from GPS */
Adafruit_PWMServoDriver servo_driver = Adafruit_PWMServoDriver();                               /**< Adafruit servo driver object */
SimpleServo strobes[] = {                                                                       /**< External LED indicator lights, controlled by PWM like a servo */
    SimpleServo(STROBE_PWM_OFF, STROBE_PWM_ON, STROBE_CHANNEL_0, &servo_driver),
    SimpleServo(STROBE_PWM_OFF, STROBE_PWM_ON, STROBE_CHANNEL_1, &servo_driver),
    SimpleServo(STROBE_PWM_OFF, STROBE_PWM_ON, STROBE_CHANNEL_2, &servo_driver),
    SimpleServo(STROBE_PWM_OFF, STROBE_PWM_ON, STROBE_CHANNEL_3, &servo_driver)
};
SimpleMusic buzzer(BUZZER);                                                                     /**< Buzzer music object to play tunes */

uint8_t node_id_ = 1;
uint8_t node_type_ = NODE_TYPES::NODE_TYPE_BALLOON;

MissionState mission_state;                     /**< Mission state state machine object */
Timer timer_telemetry_check;                    /**< Timer sets interval between checking telemetry */
Timer timer_telemetry_report;                   /**< Timer sets interval between reporting telemetry */
bool timer_telemetry_report_override = false;   /**< Manual interval override flag (for command_set_report_interval)*/
Timer timer_telemetry_log;                      /**< Timer sets interval between logging telemetry */
Timer timer_execution_led;                      /**< Timer sets intercal between run led blinks */

const String telemetry_log_name = "tlm.csv";
const String telemetry_log_header = "ts,lat,lon,alt,alt_elpd,alt_rel,alt_baro,vel_vert,vel_hor,roll,pitch,hdng,crs,temp,pres,hdop,fix";

/**
 * @brief System setup function
 * @details Initialises all system componenets at start-up
 */
void setup() {
    //Sleep until debug can connect
    //while(!Serial);

    //Setup pin modes
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_STATUS_R, OUTPUT);
    pinMode(LED_STATUS_G, OUTPUT);
    pinMode(LED_STATUS_B, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    
    //Start debug serial port
    logger.init();
    logger.event(LOG_LEVELS::INFO, "HAB systems starting...");

    //Start RTC
    logger.event(LOG_LEVELS::INFO, "Starting Real Time Clock...");
    if (!rtc.begin())
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Real Time Clock!");
        stop();
    }
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Set RTC if power was lost
    logger.event(LOG_LEVELS::INFO, "Checking real time clock status...");
    if (rtc.lostPower())
    {
        logger.event(LOG_LEVELS::WARNING, "RTC lost power, setting new date and time!");

        //Sets to when the sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

        update_rtc_from_gps = true;
    }
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Start radio modem Serial port
    logger.event(LOG_LEVELS::INFO, "Starting radio modem serial port...");
    static_cast<HardwareSerial&>(radio_input_output_stream).begin(57600);
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Start cellular modem Serial port
    logger.event(LOG_LEVELS::INFO, "Starting cellular modem serial port...");
    static_cast<HardwareSerial&>(cellular_input_output_stream).begin(57600);
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Start APRS Serial Port
    logger.event(LOG_LEVELS::INFO, "Starting APRS radio serial port...");
    static_cast<HardwareSerial&>(aprs_output_stream).begin(9600);
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Setup Sercoms
    //!!!NOTE!!! These MUST be executed AFTER "beginning" the serial port!
    pinPeripheral(3, PIO_SERCOM_ALT);
    pinPeripheral(4, PIO_SERCOM_ALT);
    pinPeripheral(10, PIO_SERCOM);
    pinPeripheral(11, PIO_SERCOM);

    //Initialise state machine
    logger.event(LOG_LEVELS::INFO, "Initialising Mission State subsystem...");
    if(!mission_state.set(MISSION_STATES::STAGING))
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Mission State subsystem!");
        stop();
    }
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Initialise the telemetry system
    logger.event(LOG_LEVELS::INFO, "Initialising Telemetry subsystem...");
    if(!telemetry.init())
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Telemetry subsystem!");
        stop();
    }
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Start telemetry data logger
    logger.event(LOG_LEVELS::INFO, "Initialising Telemetry data logger...");
    if(!telemetry_logger.init(telemetry_log_name, telemetry_log_header))
    {
        logger.event(LOG_LEVELS::FATAL, "Failed to initialise Telemetry data logger!");
        stop();
    }
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Initialize PWM driver and servos
    logger.event(LOG_LEVELS::INFO, "Starting PWM Driver...");
    servo_driver.begin();
    servo_driver.setPWMFreq(SERVO_PWM_FREQUENCY);
    logger.event(LOG_LEVELS::INFO, "Done!");

    //Initialize the watchdog
    Watchdog.enable(WATCHDOG_TIMEOUT);

    logger.event(LOG_LEVELS::INFO, "Finished initialisation, starting program!");
}

/**
 * @brief Main program loop
 * @details Called after setup() function, loops inifiteley, everything happens here
 */
void loop() {
    MissionStateFunction current_mission_state_function;            /**< Current mission state function */
    SimpleUtils::TelemetryStruct current_telemetry;                 /**< Current telemetry */
    timer_telemetry_check.setInterval(TELEMETRY_CHECK_INTERVAL);    /**< Sets interval for telemetry check timer */
    timer_execution_led.setInterval(EXECUTION_LED_INTERVAL);        /**< Sets execution blinky LED interval */
    unsigned long time_of_last_error = millis();
    char gps_string[GPS_SERIAL_BUFFER_SIZE];
    int gps_string_chars = 0;
    int gps_string_position = 0;

    //Set initial program timers
    mission_state.getFunction(current_mission_state_function);
    setTimers(current_mission_state_function);

    //Start system timers
    timer_execution_led.start();
    timer_telemetry_check.start();
    timer_telemetry_log.start();
    timer_telemetry_report.start();

    while(1)
    {
        //Update telemetry first
        logger.event(LOG_LEVELS::DEBUG, "Running telemetry system update");
        telemetry.update();

        //Push GPS chars to APRS Radio
        logger.event(LOG_LEVELS::DEBUG, "Pushing GPS string to APRS radio");
        gps_string_chars = telemetry.getGpsString(gps_string, sizeof(gps_string)/sizeof(char));
        gps_string_position = 0;
        while(gps_string_position < gps_string_chars)
        {
            aprs_output_stream.write(gps_string[gps_string_position]);
            gps_string_position++;
        }

        //Get messages from command interfaces
        logger.event(LOG_LEVELS::DEBUG, "Processing messages from command interfaces");
        radio.receive();
        cellular.receive();

        //Get the latest Telemetry
        logger.event(LOG_LEVELS::DEBUG, "Checking telemetry timer");
        if(timer_telemetry_check.check())
        {
            //Get latest telemetry
            logger.event(LOG_LEVELS::DEBUG, "Getting update from Telemetry subsystem.");

            if(!telemetry.get(current_telemetry))
            {
                logger.event(LOG_LEVELS::ERROR, "Failed to get update from Telemetry subsystem!");
                time_of_last_error = millis();
            }
            else
            {
                logger.event(LOG_LEVELS::DEBUG, "Telemetry update completed.");
                timer_telemetry_check.reset();
            }
        }

        //Set RTC if needed and gps fix is true
        logger.event(LOG_LEVELS::DEBUG, "Checking for RTC update from GPS");
        if (update_rtc_from_gps && telemetry.getGpsFixStatus()) 
        {
            logger.event(LOG_LEVELS::INFO, "Setting RTC from GPS timestamp.");

            // This line sets the RTC with an explicit date & time from unix epoch
            rtc.adjust(DateTime(telemetry.getGpsUnixTime()));

            update_rtc_from_gps = false;

            logger.event(LOG_LEVELS::INFO, "RTC was adjusted from GPS timestamp.");
        }

        //Telemetry Log
        logger.event(LOG_LEVELS::DEBUG, "Checking telemetry log timer");
        if(timer_telemetry_log.check())
        {
            logger.event(LOG_LEVELS::DEBUG, "Logging telemetry to storage.");
            logTelemetry(current_telemetry);

            timer_telemetry_log.reset();
        }

        //Telemetry Report
        logger.event(LOG_LEVELS::DEBUG, "Checking telemetry report timer");
        if(timer_telemetry_report.check())
        {
            logger.event(LOG_LEVELS::INFO, "Sending telemetry report message.");
            sendReportTelemetry(current_telemetry);

            timer_telemetry_report.reset();
        }

        //Buzzer Beeper
        logger.event(LOG_LEVELS::DEBUG, "Checking buzzer enable status");
        if(current_mission_state_function.beeper_enabled)
        {
            logger.event(LOG_LEVELS::DEBUG, "Playing buzzer");
            buzzer.play();
        }
        else
        {
            logger.event(LOG_LEVELS::DEBUG, "Stopping buzzer");
            buzzer.stop();
        }

        //Strobe Blinker
        logger.event(LOG_LEVELS::DEBUG, "Checking strobe enable status");
        if(current_mission_state_function.strobe_enabled)
        {
            logger.event(LOG_LEVELS::DEBUG, "Strobes enabled");
            for(int i = 0; i < STROBE_COUNT; i++)
            {
                strobes[i].goToMax();
            }
        }
        else
        {
            logger.event(LOG_LEVELS::DEBUG, "Strobes disabled");
            for(int i = 0; i < STROBE_COUNT; i++)
            {
                strobes[i].goToMin();
            }
        }

        //Health status LED indicator
        

        //Execution LED indicator blinkies
        logger.event(LOG_LEVELS::DEBUG, "Checking running LED timer");
        if(timer_execution_led.check())
        {
            //Reset Watchdog
            Watchdog.reset();

            //Blink LED
            if(digitalRead(LED_BUILTIN) == HIGH)
            {
                digitalWrite(LED_BUILTIN, LOW);
                digitalWrite(LED_STATUS_B, LOW);
            }
            else
            {
                digitalWrite(LED_BUILTIN, HIGH);
                digitalWrite(LED_STATUS_B, HIGH);
            }

            //Set Health LED color
            if(millis() - time_of_last_error > 1000 && digitalRead(LED_BUILTIN) == LOW)
            {
                digitalWrite(LED_STATUS_G, HIGH);
            }
            else
            {
                digitalWrite(LED_STATUS_G, LOW);
            }

            //Send Heartbeat message
            sendHeartbeat(mission_state.get());

            //Print a bunch of debug information
            logger.event(LOG_LEVELS::DEBUG, "Current GPS Latitude   ", current_telemetry.latitude);
            logger.event(LOG_LEVELS::DEBUG, "Current GPS Longitude  ", current_telemetry.longitude);
            logger.event(LOG_LEVELS::DEBUG, "Current GPS Altitude   ", current_telemetry.altitude);
            logger.event(LOG_LEVELS::DEBUG, "Current GPS Course     ", current_telemetry.course);
            logger.event(LOG_LEVELS::DEBUG, "Current GPS Velocity V ", current_telemetry.velocity_vertical);
            logger.event(LOG_LEVELS::DEBUG, "Current GPS Velocity H ", current_telemetry.velocity_horizontal);
            logger.event(LOG_LEVELS::DEBUG, "Current IMU Roll       ", current_telemetry.roll);
            logger.event(LOG_LEVELS::DEBUG, "Current IMU Pitch      ", current_telemetry.pitch);
            logger.event(LOG_LEVELS::DEBUG, "Current IMU Yaw        ", current_telemetry.yaw);
            logger.event(LOG_LEVELS::DEBUG, "Current IMU Heading    ", current_telemetry.heading);
            logger.event(LOG_LEVELS::DEBUG, "Current Baro Altitude  ", current_telemetry.altitude_barometric);
            logger.event(LOG_LEVELS::DEBUG, "Current Baro Temp      ", current_telemetry.temperature);
            logger.event(LOG_LEVELS::DEBUG, "Current Baro Pressure  ", current_telemetry.pressure);
            logger.event(LOG_LEVELS::DEBUG, "Current Baro humidity  ", current_telemetry.humidity);
            logger.event(LOG_LEVELS::DEBUG, "Current Rel Altitude   ", current_telemetry.altitude_relative);
            logger.event(LOG_LEVELS::DEBUG, "Current Mission State  ", mission_state.get());
            logger.event(LOG_LEVELS::DEBUG, "Telemetry Report Timer Started", timer_telemetry_report.isStarted());
            logger.event(LOG_LEVELS::DEBUG, "Telemetry Report Timer Set", timer_telemetry_report.isSet());
            logger.event(LOG_LEVELS::DEBUG, "Telemetry Report Timer Interval", (int)(timer_telemetry_report.getInterval()));
            logger.event(LOG_LEVELS::DEBUG, "Telemetry Log Timer Interval", (int)(timer_telemetry_log.getInterval()));

            timer_execution_led.reset();
        }

        //Update mission state
        logger.event(LOG_LEVELS::DEBUG, "Updating Mission State subsystem.");
        if(!mission_state.update(current_telemetry))
        {
            logger.event(LOG_LEVELS::ERROR, "Failed to update Mission State subsystem!");
            time_of_last_error = millis();
        }
        else
        {
            mission_state.getFunction(current_mission_state_function);
            logger.event(LOG_LEVELS::DEBUG, "Mission State subsystem update completed.");
        }

        //Update program timers based on state
        logger.event(LOG_LEVELS::DEBUG, "Setting system timers based on mission state.");
        setTimers(current_mission_state_function);
    }
}

void setTimers(MissionStateFunction function)
{
    timer_telemetry_log.setInterval(function.telemetry_log_interval);

    if(!timer_telemetry_report_override)
    {
        timer_telemetry_report.setInterval(function.telemetry_report_interval);
    }
}

void handleMessageCallback(hdlcMessage message)
{
    logger.event(LOG_LEVELS::DEBUG, "Received a message!");

    switch(message.command)
    {
        case MESSAGE_TYPES::MESSAGE_TYPE_HEARTBEAT:
            logger.event(LOG_LEVELS::DEBUG, "Received message: Heartbeat.");
            handleMessageHeartbeat(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_REPORT_TELEMETRY:
            logger.event(LOG_LEVELS::DEBUG, "Received message: Position Report.");
            handleMessageTelemetryReport(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_ARM:
            logger.event(LOG_LEVELS::DEBUG, "Received command: Arm.");
            handleMessageCommandArm(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_DISARM:
            logger.event(LOG_LEVELS::DEBUG, "Received command: Disarm.");
            handleMessageCommandDisarm(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_SET_STATE:
            logger.event(LOG_LEVELS::DEBUG, "Received command: Set State.");
            handleMessageCommandSetState(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_COMMAND_SET_REPORT_RATE:
            logger.event(LOG_LEVELS::DEBUG, "Received command: Set Report Rate.");
            handleMessageCommandSetReportRate(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_PROTO_ACK:
            logger.event(LOG_LEVELS::DEBUG, "Received protocol message: Ack.");
            handleMessageProtoAck(message);
            break;
        case MESSAGE_TYPES::MESSAGE_TYPE_PROTO_NACK:
            logger.event(LOG_LEVELS::DEBUG, "Received protocol message: Nack.");
            handleMessageProtoNack(message);
            break;
    }
}

void handleMessageHeartbeat(hdlcMessage& message)
{
    logger.event(LOG_LEVELS::DEBUG, "Ignoring heartbeat message.");
}

void handleMessageTelemetryReport(hdlcMessage& message)
{
    logger.event(LOG_LEVELS::WARNING, "Ignoring telemetry report message.");
}

void handleMessageCommandArm(hdlcMessage& message)
{

}

void handleMessageCommandDisarm(hdlcMessage& message)
{

}

void handleMessageCommandSetState(hdlcMessage& message)
{

}

void handleMessageCommandSetReportRate(hdlcMessage& message)
{
    smpMessageCommandSetReportRate command;
    smpMessageCommandSetReportRateDecode(message, command);

    switch(command.report.value)
    {
        case MESSAGE_TYPE_REPORT_TELEMETRY:
            if(command.rate.value > 0)
            {
                timer_telemetry_report.setInterval(1000 / command.rate.value);
                timer_telemetry_report_override = true;
            }
            else
            {
                timer_telemetry_report_override = false;
            }
            break;
    }

    sendAck((MESSAGE_TYPES)message.command);
}

void handleMessageProtoAck(hdlcMessage& message)
{

}

void handleMessageProtoNack(hdlcMessage& message)
{

}

void sendHeartbeat(MISSION_STATES mission_state)
{
    hdlcMessage message;
    smpMessageHeartbeat heartbeat;

    heartbeat.state.value = (uint8_t)mission_state;

    smpMessageHeartbeatEncode(node_id_, node_type_, heartbeat, message);

    radio.send(message);
    cellular.send(message);
}

void sendReportTelemetry(SimpleUtils::TelemetryStruct& telemetry)
{
    hdlcMessage message;
    smpMessageReportTelemetry telemetry_report;

    telemetry_report.latitude.value = telemetry.latitude;
    telemetry_report.longitude.value = telemetry.longitude;
    telemetry_report.altitude.value = telemetry.altitude;
    telemetry_report.altitude_ellipsoid.value = telemetry.altitude_ellipsoid;
    telemetry_report.altitude_relative.value = telemetry.altitude_relative;
    telemetry_report.altitude_barometric.value = telemetry.altitude_barometric;
    telemetry_report.velocity_horizontal.value = telemetry.velocity_horizontal;
    telemetry_report.velocity_vertical.value = telemetry.velocity_vertical;
    telemetry_report.roll.value = telemetry.roll;
    telemetry_report.pitch.value = telemetry.pitch;
    telemetry_report.yaw.value = telemetry.yaw;
    telemetry_report.heading.value = telemetry.heading;
    telemetry_report.course.value = telemetry.course;

    smpMessageReportTelemetryEncode(node_id_, node_type_, telemetry_report, message);

    radio.send(message);
    cellular.send(message);
}

void sendAck(MESSAGE_TYPES command)
{
    hdlcMessage message;
    smpMessageProtoAck ack;

    ack.command.value = command;

    smpMessageProtoAckEncode(node_id_, node_type_, ack, message);

    radio.send(message);
    cellular.send(message);
}

void sendNack(MESSAGE_TYPES command)
{
    hdlcMessage message;
    smpMessageProtoNack nack;

    nack.command.value = command;

    smpMessageProtoNackEncode(node_id_, node_type_, nack, message);

    radio.send(message);
    cellular.send(message);
}

void logTelemetry(SimpleUtils::TelemetryStruct& telemetry)
{
    int telemetry_size = 16;
    float temp_telemetry_array[telemetry_size];

    temp_telemetry_array[0] = telemetry.latitude;
    temp_telemetry_array[1] = telemetry.longitude;
    temp_telemetry_array[2] = telemetry.altitude;
    temp_telemetry_array[3] = telemetry.altitude_ellipsoid;
    temp_telemetry_array[4] = telemetry.altitude_relative;
    temp_telemetry_array[5] = telemetry.altitude_barometric;
    temp_telemetry_array[6] = telemetry.velocity_vertical;
    temp_telemetry_array[7] = telemetry.velocity_horizontal;
    temp_telemetry_array[8] = telemetry.roll;
    temp_telemetry_array[9] = telemetry.pitch;
    temp_telemetry_array[10] = telemetry.heading;
    temp_telemetry_array[11] = telemetry.course;
    temp_telemetry_array[12] = telemetry.temperature;
    temp_telemetry_array[13] = telemetry.pressure;
    temp_telemetry_array[14] = telemetry.hdop;
    temp_telemetry_array[15] = telemetry.fix;

    telemetry_logger.entry(temp_telemetry_array, telemetry_size, true);
}

void stop()
{
    //Turn off other colors
    digitalWrite(LED_STATUS_G, LOW);
    digitalWrite(LED_STATUS_B, LOW);

    Timer fail_timer;
    fail_timer.setInterval(100);
    fail_timer.start();

    //Flash red quickly
    while(1)
    {
        buzzer.play();

        if(fail_timer.check())
        {
            digitalWrite(LED_STATUS_R, !digitalRead(LED_STATUS_R));
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            fail_timer.reset();
        }
    }
}
