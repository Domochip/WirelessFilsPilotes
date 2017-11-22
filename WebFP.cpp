#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

#include "WirelessFilsPilotes.h"

#include "WebFP.h"



//-----------------------------------------------------------------------
// Timer Tick ON (Used to send Live to FP every 5 minutes)
//-----------------------------------------------------------------------
void WebFP::TimerTickON(byte fpNumber, byte liveOnDuration) {

  //DEBUG
  //Serial.print("---TimerTickON--- clock : ");Serial.println(millis()/1000);
  //Serial.print("fp : ");Serial.print(fpNumber);Serial.print(" ; duration : ");Serial.print(liveOnDuration);Serial.print(" ; nbT : ");Serial.println(_comfortTimer[fpNumber].getNumTimers());

  //if 3 or 7sec previous timer already exists then clean it
  //if (_comfortTimer[fpNumber].getNumTimers() > 1) _comfortTimer[fpNumber].deleteTimer(1);

  //Send Full Live signal
#if(MODEL_WFP>1)
  //control Positive
  delay(10);
  _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
  //control Negative
  delay(10);
  _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#else
  digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
  digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#endif

  //then create Timer for Stop
  _comfortTimer[fpNumber].setTimeout(((long)liveOnDuration) * 1000, [this, fpNumber]() {
    this->TimerTickOFF(fpNumber);
  });
}

//-----------------------------------------------------------------------
// Timer Tick OFF (Used to stop Live to FP after 3 or 7 sec)
//-----------------------------------------------------------------------
void WebFP::TimerTickOFF(byte fpNumber) {

  //DEBUG
  //Serial.print("---TimerTickOFF--- clock : ");Serial.println(millis()/1000);
  //Serial.print("fp : ");Serial.println(fpNumber);

  //Stop Full Live signal
#if(MODEL_WFP>1)
  //control Positive
  delay(10);
  _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
  //control Negative
  delay(10);
  _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#else
  digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
  digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#endif
}

//-----------------------------------------------------------------------
// Private function that change FP state (fpNumber is zero based)
//-----------------------------------------------------------------------
void WebFP::setFP(byte fpNumber, byte stateNumber, bool force) {

  //DEBUG
  //Serial.print("---setFP--- clock : ");Serial.println(millis()/1000);
  //Serial.print("fp : ");Serial.print(fpNumber);Serial.print(" ; state : ");Serial.print(stateNumber);Serial.print(" ; force : ");Serial.print(force);Serial.print(" ; nbT : ");Serial.println(_comfortTimer[fpNumber].getNumTimers());


  //if fpNumber is over (like from init) then drop
  if (fpNumber >= _modelWFP) return;

  //if stateNumber are different (otherwise do nothing and answer OK)
  if ((_fpStates[fpNumber] != stateNumber) || force) {

    //if stateNumbers are in different ranges (otherwise won't touch stuff and go direct to remember new stateNumber)
    if ((_fpStates[fpNumber] <= 10 && stateNumber > 10) ||
        (_fpStates[fpNumber] > 10 && _fpStates[fpNumber] <= 20 && (stateNumber <= 10 || stateNumber > 20)) ||
        (_fpStates[fpNumber] > 20 && _fpStates[fpNumber] <= 30 && (stateNumber <= 20 || stateNumber > 30)) ||
        (_fpStates[fpNumber] > 30 && _fpStates[fpNumber] <= 40 && (stateNumber <= 30 || stateNumber > 40)) ||
        (_fpStates[fpNumber] > 40 && _fpStates[fpNumber] <= 50 && (stateNumber <= 40 || stateNumber > 50)) ||
        (_fpStates[fpNumber] > 50 && stateNumber <= 50) ||
        force) {

      //clean up comfortTimer of current FP (Will be recreated if necessary)
      if (_comfortTimer[fpNumber].getNumTimers() > 1) _comfortTimer[fpNumber].deleteTimer(1);
      if (_comfortTimer[fpNumber].getNumTimers() > 0) _comfortTimer[fpNumber].deleteTimer(0);

      //Then apply it
      if (stateNumber <= 10) { //Arrêt (positive only)
#if(MODEL_WFP>1)
        //control Positive
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
        //control Negative
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#else
        digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
        digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#endif
      }
      else if (stateNumber <= 20) { //Hors Gel (Negative only)
#if(MODEL_WFP>1)
        //control Positive
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        //control Negative
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#else
        digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#endif
      }
      else if (stateNumber <= 30) { //Eco (Full)
#if(MODEL_WFP>1)
        //control Positive
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
        //control Negative
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#else
        digitalWrite(_FPPINMAP[fpNumber * 2], LOW);
        digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], LOW);
#endif
      }
      else if (stateNumber <= 40) { //Confort-2 (Full Live during 7s Every 5min)
#if(MODEL_WFP>1)
        //Switch OFF
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#else
        digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#endif

        //Setup Timer system
        _comfortTimer[fpNumber].setInterval(300000L, [this,  fpNumber]() {
          //_comfortTimer[fpNumber].setInterval(30000L, [this,  fpNumber]() { //DEBUG
          this->TimerTickON(fpNumber, 7);
        });
      }
      else if (stateNumber <= 50) { //Confort-1 (Full Live during 3s Every 5min)

#if(MODEL_WFP>1)
        //Switch OFF
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#else
        digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#endif

        //Setup Timer system
        _comfortTimer[fpNumber].setInterval(300000L, [this,  fpNumber]() {
          //_comfortTimer[fpNumber].setInterval(30000L, [this,  fpNumber]() { //DEBUG
          this->TimerTickON(fpNumber, 3);
        });
      }
      else { //Confort (OFF/Nothing)
#if(MODEL_WFP>1)
        //control Positive
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        //control Negative
        delay(10);
        _mcp23017.digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#else
        digitalWrite(_FPPINMAP[fpNumber * 2], HIGH);
        digitalWrite(_FPPINMAP[(fpNumber * 2) + 1], HIGH);
#endif
      }
    }

    //Remember state passed
    _fpStates[fpNumber] = stateNumber;

    //save new value in SPIFFS file
    //force is used while init and so data is coming from file and so don't need to write it back to file
    if (!force) {

      File FPStateFile = SPIFFS.open("/FPState.bin", "w");
      if (FPStateFile) {

        //write variable into file
        int nbWritten = FPStateFile.write(_fpStates, 8);
        //close file
        FPStateFile.close();

        if (nbWritten != 8) SPIFFS.remove("/FPState.bin");
      }
    }
  }
}

//------------------------------------------
//return WebFP States
String WebFP::GetStatus() {

  String statusJSON('{');
  for (byte i = 0; i < _modelWFP; i++) {
    statusJSON = statusJSON + (i ? "," : "") + F("\"FP") + (i + 1) + F("\":") + _fpStates[i];
  }
  statusJSON += '}';

  return statusJSON;
}



//------------------------------------------
//Function to initiate FilsPilote
void WebFP::Init() {

#if(MODEL_WFP>1)
  //init I2C and MCP23017
  _mcp23017.begin();
  delay(10);

  //Set port as output
  for (byte i = 0; i < 16; i++) {
    _mcp23017.pinMode(i, OUTPUT);
    delay(10);
  }

  _mcp23017.writeGPIOAB(0);

#else
  //init pin to output
  pinMode(_FPPINMAP[0], OUTPUT);
  digitalWrite(_FPPINMAP[0], HIGH);
  pinMode(_FPPINMAP[1], OUTPUT);
  digitalWrite(_FPPINMAP[1], HIGH);
#endif

  //init SPIFFS
  if (!SPIFFS.begin()) {
    SPIFFS.format();
    SPIFFS.begin();
  }
  if (SPIFFS.exists("/FPState.bin")) {
    File FPStateFile = SPIFFS.open("/FPState.bin", "r");
    if (FPStateFile && (FPStateFile.size() == 8)) {

      //load file content in variable
      FPStateFile.readBytes((char*)_fpStates, 8);

      //close file
      FPStateFile.close();

      //set FP state
      for (byte i = 0; i < 8; i++) {
        setFP(i, _fpStates[i], true);
        delay(200);
      }
    }
  }

  _initialized = true;
}

//------------------------------------------
//Run for Timers
void WebFP::Run() {
  for (byte i = 0; i < _modelWFP; i++) _comfortTimer[i].run();
}


void WebFP::InitWebServer(AsyncWebServer &server) {

  server.on("/setFP", HTTP_GET, [this](AsyncWebServerRequest * request) {

    byte fpPassed = 0;
    byte fpValues[8];


    char paramName[4] = {'F', 'P', '1', 0};
    //for each FP
    for (byte i = 0; i < _modelWFP; i++) {

      //build paramName
      paramName[2] = '1' + i;

      //check FP param is there
      if (request->hasParam(paramName)) {

        //convert fpValue
        int fpValue = request->getParam(paramName)->value().toInt();

        //check value is correct
        if ((fpValue != 0 || request->getParam(paramName)->value() == "0") && fpValue < 100) {

          //put it in table
          fpPassed += (1 << i);
          fpValues[i] = fpValue;
        }
      }
    }

    //if no fp order passed
    if (!fpPassed) {
      //answer with error and return
      request->send(400, F("text/html"), F("No valid order received"));
      return;
    }

    //for each FP
    for (byte i = 0; i < _modelWFP; i++) {
      //fp has been passed then apply
      if (fpPassed & (1 << i)) setFP(i, fpValues[i]);
    }

    request->send(200);
  });

  server.on("/gs1", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetStatus());
  });
}

