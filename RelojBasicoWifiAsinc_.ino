/*
   Projecto reloj papi chulo
   -------------------------
   © borislasky@gmail.com

   Comienzo junio 2019

   Tiene dos partes de visualización:
   - Hora
   - Otra información

   Qué hace
   - Se conecta a la wifi
   - Se conecta a un servidor NTP
   - Se conecta a un broker MQTT que le dirá qué info complementaria ofrece:
      - 00. Fecha. Default. Seteo al principio y actualizaré a las 00:00:00
      - 01. Temperatura actual, máxima y mínima
      - 02. Humedad
      - 03. Presión
      - 04. Estado del cielo
      - 05. Viento: velocidad y dirección
      - timestamp. Con esto compruebo si los datos meteos que tengo son viejos
        o no

      Los mensajes MQTT vendrán siempre todos. Como los publico con retained,
      los tendré siempre. Cuando quiera visualizar alguno, si es viejo,
      pediré al node-red que los actualice y me llegarán via MQTT.

      Vuelvo a default pasado un tiempo

   - Un pulsador irá cambiando la función a visulaizar. También vuelve a
   default.
*/

/*
   SCL D1
   SDA D2
   MUX0 RTC
   MUX1 Reloj
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
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
// MQTT
//------------------------------------------------------------------------------
PubSubClient MQTTclient(InternetClient);
//IPAddress    MQTT_SERVER_IP;
#define MQTT_SERVER     "192.168.43.212"
#define MQTT_PORT       1883
#define MQTT_USER       "pi"
#define MQTT_PASSWORD   "picom222"
#define MQTT_IN_FUNCION "/torredembarra/reloj/0/funcion/set"
#define MQTT_IN_DATOS   "/torredembarra/DatosMeteo/#"
#define MQTT_CLIENT_ID  "ESP-00-/torredembarra/reloj/0/funcion/"
/*
  const char*  MQTT_SERVER     = "192.168.43.212";
  const int    MQTT_PORT       = 1883;
  const char*  MQTT_USER       = "pi";
  const char*  MQTT_PASSWORD   = "picom222";
  const char*  MQTT_IN_FUNCION = "/torredembarra/reloj/0/funcion/set";
  const char*  MQTT_IN_DATOS   = "/torredembarra/DatosMeteo/#";
  const char*  MQTT_CLIENT_ID  = "ESP-00-/torredembarra/reloj/0/funcion/";
*/

//------------------------------------------------------------------------------
// NTP
//------------------------------------------------------------------------------
//#define NTP_ADDRESS  "192.168.43.200"  // URL NTP laskyserver
//#define NTP_ADDRESS  "192.168.43.212"  // URL NTP mini
//#define NTP_ADDRESS  "poll.ntp.org"  // URL NTP
//#define NTP_ADDRESS  "laskyserver.ddns.net"  // URL NTP
#define NTP_ADDRESS  "192.168.43.202"  // URL NTP lasky
#define NTP_OFFSET   0
// En horas. Teóricamente el offset lo setea
// timezone.h
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


//------------------------------------------------------------------------------
// Ticker para activar la vuelta a default
//------------------------------------------------------------------------------
Ticker VuelveDefaultT;
#define VuelveDefault 15  // 15 segundos


//------------------------------------------------------------------------------
// Variables globales para visualización
//------------------------------------------------------------------------------
#define SHOW_TIME_PERIOD 1000
const char * dias[]  = {"Domingo", "Lunes", "Martes", "Miercoles",
                        "Jueves", "Viernes", "Sabado"
                       } ;
const char * meses[] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                        "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"
                       } ;

//------------------------------------------------------------------------------
// Variables globales de estado
//------------------------------------------------------------------------------
#define viejo           15 * 60 // >15 minutos lo considero viejo
#define TotalFunciones  6
int     Funcion = 0; // qué tengo que visualizar en el panel de información
String  FechaHora, detalle, tempExt, tempInt, humedadExt, humedadInt,
        viento, amanecer, anochecer;
unsigned int timestamp = 0; // guarda el timestamp de cuando recibo datos
unsigned int PendPintarSalida = false;
// Lo utilizo cuando tengo datos viejos y
// estoy pendiente de recibir nuevos

//------------------------------------------------------------------------------
// Pulsador cambio de función
//------------------------------------------------------------------------------
const int pulsador = 2; // D4
volatile int EstaPulsado = 0;


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
