// Funciones de rtc PCF8563 cuando lo uso con multiplexor TCA9548A
// En realidad llamo a las funciones de la librería Rtc_Pcf8563.h
// llamando previamente a tcaselect con el canal que me viene en el
// constructor

#include "PCF8563_TCA9548A.h"

PCF8563_TCA9548A::PCF8563_TCA9548A(uint8_t dirMux, uint8_t canalMux){
	this->DirMux 		= dirMux;
	this->CanalMux	= canalMux;
	this->rtc = 		 rtc;
}

void PCF8563_TCA9548A::setTime(byte sec, byte minute, byte hour){
	tcaselect(this->DirMux, this->CanalMux);
	this->rtc.setTime(sec, minute, hour);
}
byte PCF8563_TCA9548A::getSecond(){
	tcaselect(this->DirMux, this->CanalMux);
	return this->rtc.getSecond();
}
byte PCF8563_TCA9548A::getMinute(){
	tcaselect(this->DirMux, this->CanalMux);
	return this->rtc.getMinute();
}
char *PCF8563_TCA9548A::formatTime(){
	tcaselect(this->DirMux, this->CanalMux);
	return this->rtc.formatTime();
}