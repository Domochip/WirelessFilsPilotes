#ifndef WebFP_h
#define WebFP_h

#include "Adafruit_MCP23017.h"
#include "SimpleTimer.h"

class WebFP {

  private:

    //Pin Map is list of pins by pair corresponding to FilsPilotes
    //{Positive of FP1,Negative of FP1,Positive of FP2,Negative of FP2,etc.,...}
#if(MODEL_WFP>1)
    const byte _FPPINMAP[16] = {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8};
#else
    const byte _FPPINMAP[2] = {5, 4};
#endif
    byte _modelWFP = MODEL_WFP;
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
    void Init();
    void Run();
    void InitWebServer(AsyncWebServer & server);
};

#endif

