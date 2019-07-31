// Funciones relacionadas con la visualización de los OLED
// con un multiplexor TCA9548A

#ifndef OLED_h
#define OLED_h

#include <Adafruit_SSD1306.h>
#include <TCA9548A.h>
#include <Wire.h>
#include <fonts.h>

#define Nfuentes 1	// Número de fuentes que trataré

class OLED{
	private:
		Adafruit_SSD1306 display;
		const GFXfont *f;
		const GFXfont *fuentes[Nfuentes];
		
	public:
		OLED(uint8_t dirMux, uint8_t canalMux, uint8_t ancho, uint8_t alto);
		// Adafruit_SSD1306 display(uint8_t a, uint8_t b, TwoWire *e, uint8_t c);
		uint8_t DirMux;			// dirección i2c del multiplexor
		uint8_t CanalMux;		// canal utilizado por el OLED
		uint8_t DirOLED;		// dirección i2c del OLED
		uint8_t Ancho;  		// en pixels
		uint8_t Alto; 			// en pixels
		uint8_t Color;
		uint8_t Tamano;
		uint8_t indFuente; 	// índice de la fuente. Por defecto la 0
		uint8_t ini();			// inicialización del OLED
		void setTamano(uint8_t tamano);
		void setColor(uint8_t color);
		void setDirOLED(uint8_t dirOLED);
		void setFont(uint8_t indice);
		void Borra();
		void Escribe(uint8_t cursorX, uint8_t cursorY, char *s, bool limpia=true, bool displaya=true); // saca por el OLED el string s con el color, fuente y tamaño definidos
		void Dibuja(uint8_t posX, uint8_t posY, const uint8_t bitmap[],
      int16_t w, int16_t h, bool limpia=true, bool displaya=true);
};

#endif
