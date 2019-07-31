# ESP8266-3xSSD1306-i2c-Clock (working)
Reloj con horas minutos y segundos, desarrollado con ESP8266, 3 OLED SSD1306, RTC PCF8563 y MUX TCA9548A  
  
## Materiales
- 1 x ESP8266
- 3 x SSD1306. i2c OLED 0.96" 128x64
- 1 x TCA9548A. Multiplexor 8 canales i2c
- 1 X PCF8563. Real Time Clock

## Principales características
- En cada uno de los OLED se muestra la hora, minutos y segundos respectivamente
- Hora obtenida por el RTC
- Actualización NTP asíncrona
- Zona horaria y horario de invierno / verano
  
## Esquemas 
- [Esquema eléctrico](https://github.com/borislasky/ESP8266-3xSSD1306-i2c-Clock/blob/develop/schemes/Schematic_Reloj_Esquema(provisional).pdf)
- PCB
