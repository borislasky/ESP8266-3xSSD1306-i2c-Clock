// Funciones relacionadas con los OLED
#include "OLED.h"

OLED::OLED(uint8_t dirMux, uint8_t canalMux, uint8_t ancho, uint8_t alto){
	this->DirMux		= dirMux;
	this->CanalMux 	= canalMux;
	this->Ancho			= ancho;
	this->Alto			= alto;
	this->Color			= WHITE;
	this->Tamano		= 1;
	this->DirOLED		= 0x3C;
	Adafruit_SSD1306 display(ancho, alto);
	this->display = display;
	this->indFuente = 0;
	this->fuentes[0] = &DSEG7Modern_Regular12pt7b;
	this->f	= fuentes[this->indFuente]; //DSEG7Modern_Regular12pt7b;
}



uint8_t OLED::ini(){
	tcaselect(this->DirMux, this->CanalMux);
	if(this->display.begin(SSD1306_SWITCHCAPVCC, DirOLED)){
		this->display.setTextColor(Color);
		display.setFont(this->f);
		this->display.setTextSize(Tamano);
		this->display.clearDisplay();
		this->display.display();
		return 1;
	}
	return 0;
}

void OLED::setTamano(uint8_t tamano){
	this->Tamano = tamano;
	tcaselect(this->DirMux, this->CanalMux);
	this->display.setTextSize(this->Tamano);
}

void OLED::setColor(uint8_t color){
	this->Color = color;
	tcaselect(this->DirMux, this->CanalMux);
	this->display.setTextColor(Color);
}

void OLED::setDirOLED(uint8_t dirOLED){
	this->DirOLED = dirOLED;
}

void OLED::setFont(uint8_t indice){
	tcaselect(this->DirMux, this->CanalMux);
	this->f = this->fuentes[indice];
	this->display.setFont(f);
}

void OLED::Borra(){
	tcaselect(this->DirMux, this->CanalMux);
	this->display.clearDisplay();
	this->display.display();
}

void OLED::Escribe(uint8_t cursorX, uint8_t cursorY, char *s, bool limpia, bool displaya){
	tcaselect(this->DirMux, this->CanalMux);
	if(limpia) this->display.clearDisplay();
	this->display.setCursor(cursorX, cursorY);
	this->display.print(s);
	if(displaya) this->display.display();
}

void OLED::Dibuja(uint8_t posX, uint8_t posY, const uint8_t bitmap[],
      int16_t w, int16_t h, bool limpia, bool displaya){
	tcaselect(this->DirMux, this->CanalMux);
	if(limpia) this->display.clearDisplay();
	this->display.drawBitmap(posX, posY, bitmap, w, h, WHITE);
	if(displaya) this->display.display();

}