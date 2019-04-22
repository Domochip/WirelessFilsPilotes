#ifndef Main_h
#define Main_h

#include <arduino.h>

//DomoChip Informations
//Configuration Web Pages :
//http://IP/
//http://IP/config
//http://IP/fw
//Fils Pilotes Request Web Pages
//http://IP/setFP?FP1=21

/*
  FilsPilotes States :
  - 0-10 : Arrêt
  - 11-20 : Hors Gel
  - 21-30 : Eco
  - 31-40 : Confort-2
  - 41-50 : Confort-1
  - 51-99 : Confort
*/

//include Application header file
#include "WirelessFilsPilotes.h"
#include "WebDS18B20Bus.h"

//model is 1, 4 or 8
#define MODEL_WFP 1

#if MODEL_WFP == 1
#define APPLICATION1_NAME "WFP1"
#else
#define APPLICATION1_NAME "WFP4/8"
#endif
#define APPLICATION1_DESC "DomoChip Wireless FilsPilotes"
#define APPLICATION1_CLASS WebFP

#if MODEL_WFP == 1
#define APPLICATION2_NAME "WDS18B20"
#define APPLICATION2_DESC "DomoChip Wireless DS18B20"
#define APPLICATION2_CLASS WebDS18B20Bus
#endif

#define VERSION_NUMBER "3.3.1"

#define DEFAULT_AP_SSID "WirelessFP"
#define DEFAULT_AP_PSK "PasswordFP"

//Enable developper mode (fwdev webpage and SPIFFS is used)
#define DEVELOPPER_MODE 0

//Choose Serial Speed
#define SERIAL_SPEED 115200

#if MODEL_WFP > 1
//Choose Pin used to boot in Rescue Mode
#define RESCUE_BTN_PIN 16
#endif

//Status LED
//red GPIO14 (LOW level light UP)
//green GPIO12 (LOW level light UP)
#define STATUS_LED_SETUP pinMode(14, OUTPUT);pinMode(12, OUTPUT);
#define STATUS_LED_OFF digitalWrite(14, HIGH);digitalWrite(12, HIGH);
#define STATUS_LED_ERROR digitalWrite(14, LOW);digitalWrite(12, HIGH);
#define STATUS_LED_WARNING digitalWrite(14, LOW);digitalWrite(12, LOW);
#define STATUS_LED_GOOD digitalWrite(14, HIGH);digitalWrite(12, LOW);

//construct Version text
#if DEVELOPPER_MODE
#define VERSION VERSION_NUMBER "-DEV"
#else
#define VERSION VERSION_NUMBER
#endif

#endif


