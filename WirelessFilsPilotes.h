#ifndef WirelessFilsPilotes_h
#define WirelessFilsPilotes_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MCP23017.h>

#include "SimpleTimer.h"

#include "Main.h"
#include "src\Utils.h"


const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

//Structure of Application Data 1
class AppData1 {

  public:
    //FP Names/Aliases
    char fpNames[8][25];

    void SetDefaultValues() {
      fpNames[0][0] = 0;
      fpNames[1][0] = 0;
      fpNames[2][0] = 0;
      fpNames[3][0] = 0;
      fpNames[4][0] = 0;
      fpNames[5][0] = 0;
      fpNames[6][0] = 0;
      fpNames[7][0] = 0;
    }

    String GetJSON();
    bool SetFromParameters(AsyncWebServerRequest* request, AppData1 &tempAppData);
};


class WebFP {

  private:
    AppData1* _appData1;

    //Pin Map is list of pins by pair corresponding to FilsPilotes
    //{Positive of FP1,Negative of FP1,Positive of FP2,Negative of FP2,etc.,...}
#if(MODEL_WFP>1)
    const byte _FPPINMAP[16] = {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8};
#else
    const byte _FPPINMAP[2] = {5, 4};
#endif
    bool _initialized = false;
#if(MODEL_WFP>1)
    Adafruit_MCP23017 _mcp23017;
#endif
    SimpleTimer _comfortTimer[8]; //8 SimpleTimer Object with max 2 timers inside (SimpleTimer.h)
    byte _fpStates[8] = {51, 51, 51, 51, 51, 51, 51, 51};

    void TimerTickON(byte fpNumber, byte liveOnDuration);
    void TimerTickOFF(byte fpNumber);

    void setFP(byte fpNumber, byte stateNumber, bool force = false);

    String GetStatus();

  public:
    void Init(AppData1 &appData1);
    void InitWebServer(AsyncWebServer &server);
    void Run();
};

#endif
