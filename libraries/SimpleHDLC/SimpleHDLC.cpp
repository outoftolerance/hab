#include "SimpleHDLC.h"

#define low(x)    ((x) & 0xFF)
#define high(x)   (((x)>>8) & 0xFF)

/*------------------------------Constructor Methods------------------------------*/

SimpleHDLC::SimpleHDLC(Stream& input_stream, message_callback_type callback_function):
	data_stream_(input_stream),
	handleMessageCallback_(callback_function)
{
	frame_position_ = 0;
    frame_crc_ = CRC16_CCITT_INIT_VAL;
    escape_byte_ = false;
}

/*------------------------------Private Methods------------------------------*/

void SimpleHDLC::sendByte_(uint8_t data)
{
	data_stream_.write(data);
}

void SimpleHDLC::serializeMessage_(const hdlcMessage& message, uint8_t buffer[], uint8_t buffer_length)
{
	for(int i = 0; i < message.length; i++)
	{
		if(i == 0)
		{
			buffer[i] = message.command;
		}
		else if (i == 1)
		{
			buffer[i] = message.length;
		}
		else
		{
			buffer[i] = message.payload[i - 2];
		}
	}
}

void SimpleHDLC::deserializeMessage_(hdlcMessage& message, const uint8_t buffer[], uint8_t buffer_length)
{
	for(int i = 0; i < buffer_length; i++)
	{
		if(i == 0)
		{
			message.command = buffer[i];
		}
		else if(i == 1)
		{
			message.length = buffer[i];
		}
		else
		{
			message.payload[i - 2] = buffer[i];
		}
	}
}

/*------------------------------Public Methods------------------------------*/

void SimpleHDLC::receive()
{
	uint8_t new_byte;

	//Loop through all the available bytes in the serial port
	while(data_stream_.available() > 0)
	{
		//Read new byte from data_stream_
		new_byte = data_stream_.read();

		//Check for start of new frame
		if(new_byte == FRAME_FLAG)
		{
			if(escape_byte_ == true)
			{
				escape_byte_ = false;
			}
			//A valid frame has been found
			else if( (frame_position_ >= 2) && ( frame_crc_ == (uint8_t)((frame_receive_buffer_[frame_position_ - 1] << 8 ) | (frame_receive_buffer_[frame_position_ - 2] & 0xff)) ) )  // (msb << 8 ) | (lsb & 0xff)
			{
				//Decode new frame into message
				hdlcMessage new_message;
				deserializeMessage_(new_message, frame_receive_buffer_, (uint8_t)(frame_position_ - 2));

				//Execute message callback function
				(*handleMessageCallback_)(new_message);
			}

			//Start of a new frame! Reset CRC and position
			frame_position_ = 0;
			frame_crc_ = CRC16_CCITT_INIT_VAL;
			continue;
		}

		//Check if we need to escape a byte
		if(escape_byte_)
		{
			escape_byte_ = false;
			new_byte ^= INVERT_BYTE;
		}
		else if(new_byte == CONTROL_ESCAPE_BYTE)
		{
			escape_byte_ = true;
			continue;
		}

		//Add the new byte to the frame receive buffer
		frame_receive_buffer_[frame_position_] = new_byte;

		//Update the CRC if we are at last 2 positions into the frame
		if(frame_position_-2 >= 0) 
		{
			frame_crc_ = _crc_ccitt_update(frame_crc_, frame_receive_buffer_[frame_position_-2]);
		}

		//Increment position within frame
		frame_position_++;

		//Check if we hit the max length of the frame
		if(frame_position_ == MAX_FRAME_LENGTH)
		{
			//Reset to start of frame and start again
			frame_position_ = 0;
			frame_crc_ = CRC16_CCITT_INIT_VAL;
		}
	}
}

void SimpleHDLC::send(const hdlcMessage& message)
{
	uint8_t data;
    uint16_t fcs = CRC16_CCITT_INIT_VAL;

    //Convert message to serial bytes
    uint8_t buffer[MAX_FRAME_LENGTH];
    serializeMessage_(message, buffer, message.length + 2);

    //Send initial frame flag to open frame
    sendByte_((uint8_t)FRAME_FLAG);

    //Loop through the serialized data buffer and send all bytes making sure to convert
    for(int i = 0; i < message.length + 2; i++)
    {
        data = buffer[i];
        fcs = _crc_ccitt_update(fcs, data);

        if((data == CONTROL_ESCAPE_BYTE) || (data == FRAME_FLAG))
        {
            sendByte_((uint8_t)CONTROL_ESCAPE_BYTE);
            data ^= INVERT_BYTE;
        }

        sendByte_((uint8_t)data);
    }

    //Get bottom 8 bits of CRC and send making sure to convert
    data = low(fcs);

    if((data == CONTROL_ESCAPE_BYTE) || (data == FRAME_FLAG))
    {
        sendByte_((uint8_t)CONTROL_ESCAPE_BYTE);
        data ^= (uint8_t)INVERT_BYTE;
    }

    sendByte_((uint8_t)data);

    //Get top 8 bits of CRC and send making sure to convert
    data = high(fcs);

    if((data == CONTROL_ESCAPE_BYTE) || (data == FRAME_FLAG))
    {
        sendByte_(CONTROL_ESCAPE_BYTE);
        data ^= INVERT_BYTE;
    }

    sendByte_(data);

    //Send final frame flag to close frame
    sendByte_(FRAME_FLAG);
}