// Funciones de rtc PCF8563 cuando lo uso con multiplexor TCA9548A
// En realidad llamo a las funciones de la librería Rtc_Pcf8563.h
// llamando previamente a tcaselect con el canal que me viene en el
// constructor

#ifndef PCF8563_TCA9548A_h
#define PCF8563_TCA9548A_h

#include <Rtc_Pcf8563.h>
#include <TCA9548A.h>

class PCF8563_TCA9548A{
	public:
		byte DirMux;
		byte CanalMux;
		Rtc_Pcf8563 rtc;
		PCF8563_TCA9548A(uint8_t dirMux, uint8_t canalMux);
		// pongo las funciones que necesite
		void setTime(byte sec, byte minute, byte hour);
		byte getSecond();
		byte getMinute();
		byte getHour();
		byte getDay();
		byte getMonth();
		byte getYear();
		byte getWeekday();
		char *formatTime();
};

#endif