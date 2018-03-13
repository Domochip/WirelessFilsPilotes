#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "WirelessFilsPilotes.h"


//Return JSON of AppData1 content
String AppData1::GetJSON() {

  String gc;

  bool first = true;
  for (byte i = 0; i < MODEL_WFP; i++) {
    if (first) first = false;
    else gc = gc + ',';
    gc = gc + F("\"fp") + (i + 1) + F("n\":\"") + fpNames[i] + '"';
  }

  return gc;
}

//Parse HTTP Request into an AppData1 structure
bool AppData1::SetFromParameters(AsyncWebServerRequest* request, AppData1 &tempAppData1) {

  if (request->hasParam(F("fp1n"), true) && request->getParam(F("fp1n"), true)->value().length() < sizeof(fpNames[0])) strcpy(tempAppData1.fpNames[0], request->getParam(F("fp1n"), true)->value().c_str());
  else tempAppData1.fpNames[0][0] = 0;
  if (request->hasParam(F("fp2n"), true) && request->getParam(F("fp2n"), true)->value().length() < sizeof(fpNames[1])) strcpy(tempAppData1.fpNames[1], request->getParam(F("fp2n"), true)->value().c_str());
  else tempAppData1.fpNames[1][0] = 0;
  if (request->hasParam(F("fp3n"), true) && request->getParam(F("fp3n"), true)->value().length() < sizeof(fpNames[2])) strcpy(tempAppData1.fpNames[2], request->getParam(F("fp3n"), true)->value().c_str());
  else tempAppData1.fpNames[2][0] = 0;
  if (request->hasParam(F("fp4n"), true) && request->getParam(F("fp4n"), true)->value().length() < sizeof(fpNames[3])) strcpy(tempAppData1.fpNames[3], request->getParam(F("fp4n"), true)->value().c_str());
  else tempAppData1.fpNames[3][0] = 0;
  if (request->hasParam(F("fp5n"), true) && request->getParam(F("fp5n"), true)->value().length() < sizeof(fpNames[4])) strcpy(tempAppData1.fpNames[4], request->getParam(F("fp5n"), true)->value().c_str());
  else tempAppData1.fpNames[4][0] = 0;
  if (request->hasParam(F("fp6n"), true) && request->getParam(F("fp6n"), true)->value().length() < sizeof(fpNames[5])) strcpy(tempAppData1.fpNames[5], request->getParam(F("fp6n"), true)->value().c_str());
  else tempAppData1.fpNames[5][0] = 0;
  if (request->hasParam(F("fp7n"), true) && request->getParam(F("fp7n"), true)->value().length() < sizeof(fpNames[6])) strcpy(tempAppData1.fpNames[6], request->getParam(F("fp7n"), true)->value().c_str());
  else tempAppData1.fpNames[6][0] = 0;
  if (request->hasParam(F("fp8n"), true) && request->getParam(F("fp8n"), true)->value().length() < sizeof(fpNames[7])) strcpy(tempAppData1.fpNames[7], request->getParam(F("fp8n"), true)->value().c_str());
  else tempAppData1.fpNames[7][0] = 0;

  return true;
}







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
  if (fpNumber >= MODEL_WFP) return;

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
//return WebFP Status in JSON
String WebFP::GetStatus() {

  String statusJSON('{');
  for (byte i = 0; i < MODEL_WFP; i++) {
    statusJSON = statusJSON + (i ? "," : "") + F("\"FP") + (i + 1) + F("\":") + _fpStates[i];
  }
  statusJSON += '}';

  return statusJSON;
}


//------------------------------------------
//Function to initiate WebFP with Config
void WebFP::Init(AppData1 &appData1) {

  Serial.print(F("Start WebFP"));

  _appData1 = &appData1;


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

  Serial.println(F(" : OK"));
}

//------------------------------------------
void WebFP::InitWebServer(AsyncWebServer &server) {

  server.on("/gs1", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetStatus());
  });


  server.on("/setFP", HTTP_GET, [this](AsyncWebServerRequest * request) {

    byte fpPassed = 0;
    byte fpValues[8];


    char paramName[4] = {'F', 'P', '1', 0};
    //for each FP
    for (byte i = 0; i < MODEL_WFP; i++) {

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
    for (byte i = 0; i < MODEL_WFP; i++) {
      //fp has been passed then apply
      if (fpPassed & (1 << i)) setFP(i, fpValues[i]);
    }

    request->send(200);
  });
}

//------------------------------------------
//Run for timer
void WebFP::Run() {

  for (byte i = 0; i < MODEL_WFP; i++) _comfortTimer[i].run();
}
