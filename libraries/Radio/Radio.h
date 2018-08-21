#ifndef Radio_h
#define Radio_h

#include <string.h>
#include <HardwareSerial.h>

namespace Radio
{
	/**
	 * @brief      Radio class, abstracts different radio types
	 */
	class Radio
	{
		public:
			Radio(HardwareSerial* serial);
			~Radio();
			virtual write() = 0;
		private:

		protected:
			HardwareSerial* serial_port_;
	}
}

#endif