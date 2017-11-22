#ifndef WirelessFilsPilotes_h
#define WirelessFilsPilotes_h


//DomoChip Informations
//------------Compile for 1M 64K SPIFFS------------
//Configuration Web Pages
//http://IP/fw
//http://IP/config
//http://IP/status
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

#define VERSION_NUMBER "3.2.0"

//model is 1, 4 or 8
#define MODEL_WFP 8

#if MODEL_WFP==1
#define MODEL "WFP1"
#else
#define MODEL "WFP4/8"
#endif

//Enable developper mode (SPIFFSEditor is used)
#define DEVELOPPER_MODE 0

//Choose Serial Speed
#define SERIAL_SPEED 115200

//Choose Pin used to boot in Rescue Mode
#define RESCUE_BTN_PIN 16

//construct Version text
#if DEVELOPPER_MODE
#define VERSION VERSION_NUMBER "-DEV"
#else
#define VERSION VERSION_NUMBER
#endif

#endif


