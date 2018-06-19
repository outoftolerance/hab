#include "Communications.h"

/*------------------------------Constructor Methods------------------------------*/

Communications::Communications()
{

}

Communications::Communications(HardwareSerial* terminal_0, HardwareSerial* terminal_1)
{
	output_0_ = terminal_0;
	output_1_ = terminal_1;
}

/*------------------------------Private Methods------------------------------*/

void Communications::send(TelemetryStruct* telemetry)
{
	
}