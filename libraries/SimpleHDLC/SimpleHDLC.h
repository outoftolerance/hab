#ifndef SIMPLEHDLC_H
#define SIMPLEHDLC_H

#include <Arduino.h>

#include <stdint.h>
#include <crc16.h>

#define FRAME_FLAG 0x7e /**< Message start flag */
#define CONTROL_ESCAPE_BYTE 0x7D /**< A "control escape octet", has the bit sequence '01111101', (7D hexadecimal) */
#define INVERT_BYTE 0x20 /**<  */
#define CRC16_CCITT_INIT_VAL 0xFFFF /**< Initial value for CRC checksum */
#define MAX_FRAME_LENGTH 64 /**< Maximum length of a frame including start and end flags */

/**
 * Defines the structure of the message
 */
typedef struct hdlcMessage
{
    uint8_t command;
    uint8_t length;
    uint8_t payload[MAX_FRAME_LENGTH - 8];
} hdlcMessage;

typedef void (* message_callback_type)(hdlcMessage message);

/**
 * @brief      Simple HDLC message interface class
 */
class SimpleHDLC
{
    public:
        /**
         * @brief      Constructor of SimpleHDLC object
         *
         * @param      input_stream  Pointer to the input stream the object should read from
         */
        SimpleHDLC(Stream& input_stream, message_callback_type);

        /**
         * @brief      Processes the data stream to find messages.
         */
        void receive();

        /**
         * @brief      Function to send a message via the stream, will be wrapped in HDLC frame
         *
         * @param[in]  message  The message to be sent
         */
        void send(const hdlcMessage& message);
    private:
        /**
         * @brief      Sends a single byte through the serial port
         *
         * @param[in]  data  The data bute to be sent
         */
        void sendByte_(uint8_t data);

        /**
         * @brief      Serializes an HDLC message as a series of bytes
         *
         * @param[in]   message    The message to serialize
         * @param[out]  buffer     The buffer to output serial data to
         * @param[out]  length     The length of the output buffer
         */
        void serializeMessage_(const hdlcMessage& message, uint8_t buffer[], uint8_t buffer_length);

        /**
         * @brief      Deserializes an HDLC message from a series of bytes
         *
         * @param[out] message  The message object to populate with deserialized data
         * @param[in]  buffer   The buffer to deserialize from
         * @param[in]  length   The length of the input buffer
         */
        void deserializeMessage_(hdlcMessage& message, const uint8_t buffer[], uint8_t buffer_length);

        Stream& data_stream_;                                   /**< Stream to read data from and publish data to */
        uint8_t frame_receive_buffer_[MAX_FRAME_LENGTH + 1];    /**< Buffer to receive frame data into from stream */
        uint16_t frame_crc_;                                    /**< CRC for frame **/
        uint8_t frame_position_;                                /**< Position within frame **/
        bool escape_byte_;                                      /**< Tracks if byte should be escaped **/
        message_callback_type handleMessageCallback_;           /**< User defined message handler callback function */
};

#endif