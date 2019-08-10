/*
   Projecto reloj papi chulo
   -------------------------
   © borislasky@gmail.com

   Comienzo junio 2019

   
*/

/*
   SCL D1
   SDA D2
   MUX0 RTC
   MUX1 Reloj
*/

#include <ESP8266WiFi.h>
#include <NtpClientLib.h>
#include <Timezone.h>
#include <OLED.h>
#include <PCF8563_TCA9548A.h>

//------------------------------------------------------------------------------
// WiFi
//------------------------------------------------------------------------------
WiFiClient   InternetClient;
#define  WIFI_SSID      "AndroidAP"
#define  WIFI_PASSWORD  "boris123"
//#define  WIFI_SSID      "MOVISTAR_4F77"
//#define  WIFI_PASSWORD  "45CB50D31273BB67E178"
bool WiFiPrimeraConexion = false;


//------------------------------------------------------------------------------
// NTP
//------------------------------------------------------------------------------
//#define NTP_ADDRESS  "192.168.43.200"  // URL NTP laskyserver
//#define NTP_ADDRESS  "192.168.43.212"  // URL NTP mini
//#define NTP_ADDRESS  "poll.ntp.org"  // URL NTP
//#define NTP_ADDRESS  "laskyserver.ddns.net"  // URL NTP
#define NTP_ADDRESS  "192.168.43.202"  // URL NTP lasky
#define NTP_OFFSET   0  // En horas. Teóricamente el offset lo setea timezone.h
		// Habrá que esperar al cambio de hora de invierno 2019
bool syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event


//------------------------------------------------------------------------------
// Cambio horario
//------------------------------------------------------------------------------
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 60};     // Hora de Verano de Europa Central
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 0};      // Hora Estandar de Europa Central
Timezone CE(CEST, CET);
time_t local;

//------------------------------------------------------------------------------
// Multiplexor TCA9548A
//------------------------------------------------------------------------------
#define MUX_Address 0x70 // Dirección i2c del multiplexor TCA9548A

//------------------------------------------------------------------------------
// RTC PCF8563 con multiplexor TCA9548A
//------------------------------------------------------------------------------
#define CANAL_RTC 0
PCF8563_TCA9548A rtc(MUX_Address, CANAL_RTC);

//------------------------------------------------------------------------------
// OLED con multiplexor TCA9548A
//------------------------------------------------------------------------------
#define CANAL_OLED_1 1
OLED segundos(MUX_Address, CANAL_OLED_1, 128, 32);

#define SHOW_TIME_PERIOD 100

// Handlers de wifi conectada e IP asignada
void HandlerWifiConectada(WiFiEventStationModeConnected ipInfo) {
  Serial.printf ("Connected to %s\r\n", ipInfo.ssid.c_str ());
}

void HandlerWifiIP(WiFiEventStationModeGotIP ipInfo) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf ("IP: %s\r\n", ipInfo.ip.toString ().c_str ());
    WiFiPrimeraConexion = true;
  }
}

// Handler de wifi desconectada
void HandlerWifiDesconectada(WiFiEventStationModeDisconnected event_info) {
  Serial.printf ("Disconnected from SSID: %s\n", event_info.ssid.c_str ());
  Serial.printf ("Reason: %d\n", event_info.reason);
  WiFiPrimeraConexion = false;
  NTP.stop();
  digitalWrite (LED_BUILTIN, HIGH); // Apaga el led de la placa
}

// Evento de sincronización NTP
void processSyncEvent (NTPSyncEvent_t ntpEvent) {
  if (!ntpEvent) {
    Serial.print ("Sincronizo con NTP: ");
    Serial.println (NTP.getTimeDateString (NTP.getLastNTPSync ()));
    local = CE.toLocal(now());
    rtc.setTime(hour(local), minute(local), second(local));
    digitalWrite (LED_BUILTIN, LOW); // Enciende el led de la placa
  }
  else {
    digitalWrite (LED_BUILTIN, HIGH); // Apaga el led de la placa
    Serial.println("Error sincronización NTP");
  }
}

void IniOLEDs() {
  if (!segundos.ini()) {
    Serial.println("Error inicialización OLED segundos");
    for (;;);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Inicialización OLED y saco YA hora actual según tengo en el RTC
  IniOLEDs();
  PintaHora();

  // Inicialización asíncrona WiFi
  static WiFiEventHandler e1, e2, e3;
  WiFi.mode (WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  e1 = WiFi.onStationModeGotIP(HandlerWifiIP);
  e2 = WiFi.onStationModeDisconnected(HandlerWifiDesconectada);
  e3 = WiFi.onStationModeConnected(HandlerWifiConectada);

  // Inicialización asíncrona NTP
  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });
}


void loop() {
  static int last = 0;

  if (WiFiPrimeraConexion) {
    WiFiPrimeraConexion = false;
    NTP.setNTPTimeout(1000);  // Timeout de respuesta del servidor en segundos
    NTP.begin(NTP_ADDRESS, NTP_OFFSET, true, 0);
  }

  if (syncEventTriggered) {
    processSyncEvent(ntpEvent);
    syncEventTriggered = false;
  }

  int milis = millis();
  if ((milis - last) > SHOW_TIME_PERIOD) {
    last = milis;
    if (rtc.getMinute() == 0 && rtc.getSecond() == 0 && WiFi.status() == WL_CONNECTED) {
      NTP.getTime();
    }
    PintaHora();
  }
}


void PintaHora() {
  segundos.Escribe(0, segundos.Alto-2, rtc.formatTime());
}
