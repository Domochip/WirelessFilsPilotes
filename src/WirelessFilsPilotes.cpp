#include "WirelessFilsPilotes.h"

//-----------------------------------------------------------------------
// Timer Tick ON (Used to send Live to FP every 5 minutes)
//-----------------------------------------------------------------------
void WebFP::TimerTickON(byte fpNumber, byte liveOnDuration)
{

//DEBUG
//Serial.print("---TimerTickON--- clock : ");Serial.println(millis()/1000);
//Serial.print("fp : ");Serial.print(fpNumber);Serial.print(" ; duration : ");Serial.print(liveOnDuration);Serial.print(" ; nbT : ");Serial.println(_comfortTimer[fpNumber].getNumTimers());

//if 3 or 7sec previous timer already exists then clean it
//if (_comfortTimer[fpNumber].getNumTimers() > 1) _comfortTimer[fpNumber].deleteTimer(1);

//Send Full Live signal
#if (MODEL_WFP > 1)
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
void WebFP::TimerTickOFF(byte fpNumber)
{

//DEBUG
//Serial.print("---TimerTickOFF--- clock : ");Serial.println(millis()/1000);
//Serial.print("fp : ");Serial.println(fpNumber);

//Stop Full Live signal
#if (MODEL_WFP > 1)
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
void WebFP::setFP(byte fpNumber, byte stateNumber, bool force)
{

  //DEBUG
  //Serial.print("---setFP--- clock : ");Serial.println(millis()/1000);
  //Serial.print("fp : ");Serial.print(fpNumber);Serial.print(" ; state : ");Serial.print(stateNumber);Serial.print(" ; force : ");Serial.print(force);Serial.print(" ; nbT : ");Serial.println(_comfortTimer[fpNumber].getNumTimers());

  //if fpNumber is over (like from init) then drop
  if (fpNumber >= MODEL_WFP)
    return;

  //if stateNumber are different (otherwise do nothing and answer OK)
  if ((_fpStates[fpNumber] != stateNumber) || force)
  {

    //if stateNumbers are in different ranges (otherwise won't touch stuff and go direct to remember new stateNumber)
    if ((_fpStates[fpNumber] <= 10 && stateNumber > 10) ||
        (_fpStates[fpNumber] > 10 && _fpStates[fpNumber] <= 20 && (stateNumber <= 10 || stateNumber > 20)) ||
        (_fpStates[fpNumber] > 20 && _fpStates[fpNumber] <= 30 && (stateNumber <= 20 || stateNumber > 30)) ||
        (_fpStates[fpNumber] > 30 && _fpStates[fpNumber] <= 40 && (stateNumber <= 30 || stateNumber > 40)) ||
        (_fpStates[fpNumber] > 40 && _fpStates[fpNumber] <= 50 && (stateNumber <= 40 || stateNumber > 50)) ||
        (_fpStates[fpNumber] > 50 && stateNumber <= 50) ||
        force)
    {

      //clean up comfortTimer of current FP (Will be recreated if necessary)
      if (_comfortTimer[fpNumber].getNumTimers() > 1)
        _comfortTimer[fpNumber].deleteTimer(1);
      if (_comfortTimer[fpNumber].getNumTimers() > 0)
        _comfortTimer[fpNumber].deleteTimer(0);

      //Then apply it
      if (stateNumber <= 10)
      { //Arrêt (positive only)
#if (MODEL_WFP > 1)
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
      else if (stateNumber <= 20)
      { //Hors Gel (Negative only)
#if (MODEL_WFP > 1)
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
      else if (stateNumber <= 30)
      { //Eco (Full)
#if (MODEL_WFP > 1)
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
      else if (stateNumber <= 40)
      { //Confort-2 (Full Live during 7s Every 5min)
#if (MODEL_WFP > 1)
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
        _comfortTimer[fpNumber].setInterval(300000L, [this, fpNumber]() {
          //_comfortTimer[fpNumber].setInterval(30000L, [this,  fpNumber]() { //DEBUG
          this->TimerTickON(fpNumber, 7);
        });
      }
      else if (stateNumber <= 50)
      { //Confort-1 (Full Live during 3s Every 5min)

#if (MODEL_WFP > 1)
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
        _comfortTimer[fpNumber].setInterval(300000L, [this, fpNumber]() {
          //_comfortTimer[fpNumber].setInterval(30000L, [this,  fpNumber]() { //DEBUG
          this->TimerTickON(fpNumber, 3);
        });
      }
      else
      { //Confort (OFF/Nothing)
#if (MODEL_WFP > 1)
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
    if (!force)
    {

      File FPStateFile = SPIFFS.open("/FPState.bin", "w");
      if (FPStateFile)
      {
        //write variable into file
        int nbWritten = FPStateFile.write(_fpStates, 8);
        //close file
        FPStateFile.close();

        if (nbWritten != 8)
          SPIFFS.remove("/FPState.bin");
      }
    }
  }
}

//------------------------------------------
//Used to initialize configuration properties to default values
void WebFP::SetConfigDefaultValues()
{
  fpNames[0][0] = 0;
  fpNames[1][0] = 0;
  fpNames[2][0] = 0;
  fpNames[3][0] = 0;
  fpNames[4][0] = 0;
  fpNames[5][0] = 0;
  fpNames[6][0] = 0;
  fpNames[7][0] = 0;
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebFP::ParseConfigJSON(JsonObject &root)
{
  char fpn[5] = {'f', 'p', '1', 'n', 0};
  for (byte i = 0; i < MODEL_WFP; i++)
  {
    fpn[2] = i + '1';
    if (root[fpn].success())
      strlcpy(fpNames[i], root[fpn], sizeof(fpNames[i]));
  }
};
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebFP::ParseConfigWebRequest(AsyncWebServerRequest *request)
{
  char fpn[5] = {'f', 'p', '1', 'n', 0};
  for (byte i = 0; i < MODEL_WFP; i++)
  {
    fpn[2] = i + '1';
    if (request->hasParam(fpn, true) && request->getParam(fpn, true)->value().length() < sizeof(fpNames[i]))
      strcpy(fpNames[i], request->getParam(fpn, true)->value().c_str());
    else
      fpNames[i][0] = 0;
  }

  return true;
};
//------------------------------------------
//Generate JSON from configuration properties
String WebFP::GenerateConfigJSON(bool forSaveFile = false)
{
  String gc('{');

  for (byte i = 0; i < MODEL_WFP; i++)
    gc = gc + (i ? "," : "") + F("\"fp") + (i + 1) + F("n\":\"") + fpNames[i] + '"';

  gc += '}';

  return gc;
};
//------------------------------------------
//Generate JSON of application status
String WebFP::GenerateStatusJSON()
{
  String gs('{');

  for (byte i = 0; i < MODEL_WFP; i++)
    gs = gs + (i ? "," : "") + F("\"FP") + (i + 1) + F("\":") + _fpStates[i];

  gs += '}';

  return gs;
};
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebFP::AppInit(bool reInit)
{

  if (reInit)
    return true;

#if (MODEL_WFP > 1)
  //init I2C and MCP23017
  _mcp23017.begin();
  delay(10);

  //Set port as output
  for (byte i = 0; i < 16; i++)
  {
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

  File FPStateFile = SPIFFS.open("/FPState.bin", "r");
  if (FPStateFile && (FPStateFile.size() == 8))
  {
    //load file content in variable
    FPStateFile.readBytes((char *)_fpStates, 8);

    //close file
    FPStateFile.close();

    //set FP state
    for (byte i = 0; i < 8; i++)
    {
      setFP(i, _fpStates[i], true);
      delay(200);
    }
  }
  else
    return false;

  return true;
};
//------------------------------------------
//Return HTML Code to insert into Status Web page
const uint8_t* WebFP::GetHTMLContent(WebPageForPlaceHolder wp){
      switch(wp){
    case status:
      return (const uint8_t*) status1htmlgz;
      break;
    case config:
      return (const uint8_t*) config1htmlgz;
      break;
    default:
      return nullptr;
      break;
  };
  return nullptr;
};
//and his Size
size_t WebFP::GetHTMLContentSize(WebPageForPlaceHolder wp){
  switch(wp){
    case status:
      return sizeof(status1htmlgz);
      break;
    case config:
      return sizeof(config1htmlgz);
      break;
    default:
      return 0;
      break;
  };
  return 0;
};
//------------------------------------------
//code to register web request answer to the web server
void WebFP::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{
  server.on("/setFP", HTTP_GET, [this](AsyncWebServerRequest *request) {

    byte fpPassed = 0;
    byte fpValues[8];

    char paramName[4] = {'F', 'P', '1', 0};
    //for each FP
    for (byte i = 0; i < MODEL_WFP; i++)
    {
      //build paramName
      paramName[2] = '1' + i;

      //check FP param is there
      if (request->hasParam(paramName))
      {
        //convert fpValue
        int fpValue = request->getParam(paramName)->value().toInt();

        //check value is correct
        if ((fpValue != 0 || request->getParam(paramName)->value() == "0") && fpValue < 100)
        {
          //put it in table
          fpPassed += (1 << i);
          fpValues[i] = fpValue;
        }
      }
    }

    //if no fp order passed
    if (!fpPassed)
    {
      //answer with error and return
      request->send(400, F("text/html"), F("No valid order received"));
      return;
    }

    //for each FP
    for (byte i = 0; i < MODEL_WFP; i++)
    {
      //fp has been passed then apply
      if (fpPassed & (1 << i))
        setFP(i, fpValues[i]);
    }

    request->send(200);
  });
};

//------------------------------------------
//Run for timer
void WebFP::AppRun()
{
  for (byte i = 0; i < MODEL_WFP; i++)
    _comfortTimer[i].run();
}

//------------------------------------------
//Constructor
WebFP::WebFP(char appId, String appName) : Application(appId, appName) {}





//----------------------------------------------------------------------
// --- DS18B20Bus Class---
//----------------------------------------------------------------------

//-----------------------------------------------------------------------
// DS18X20 Read ScratchPad command
boolean DS18B20Bus::ReadScratchPad(byte addr[], byte data[])
{

  boolean crcScratchPadOK = false;

  //read scratchpad (if 3 failures occurs, then return the error
  for (byte i = 0; i < 3; i++)
  {
    // read scratchpad of the current device
    reset();
    select(addr);
    write(0xBE); // Read ScratchPad
    for (byte j = 0; j < 9; j++)
    { // read 9 bytes
      data[j] = read();
    }
    if (crc8(data, 8) == data[8])
    {
      crcScratchPadOK = true;
      i = 3; //end for loop
    }
  }

  return crcScratchPadOK;
}
//------------------------------------------
// DS18X20 Write ScratchPad command
void DS18B20Bus::WriteScratchPad(byte addr[], byte th, byte tl, byte cfg)
{

  reset();
  select(addr);
  write(0x4E); // Write ScratchPad
  write(th);   //Th 80°C
  write(tl);   //Tl 0°C
  write(cfg);  //Config
}
//------------------------------------------
// DS18X20 Copy ScratchPad command
void DS18B20Bus::CopyScratchPad(byte addr[])
{

  reset();
  select(addr);
  write(0x48); //Copy ScratchPad
}

//------------------------------------------
// Constructor for WebDS18B20Bus that call constructor of parent class OneWire
DS18B20Bus::DS18B20Bus(uint8_t pin) : OneWire(pin){};
//------------------------------------------
// Function to initialize DS18X20 sensors
void DS18B20Bus::SetupTempSensors()
{

  byte addr[8];
  byte data[9];
  boolean scratchPadReaded;

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //while we find some devices
  while (search(addr))
  {

    //if ROM received is incorrect or not a DS1822 or DS18B20 THEN continue to next device
    if ((crc8(addr, 7) != addr[7]) || (addr[0] != 0x22 && addr[0] != 0x28))
      continue;

    scratchPadReaded = ReadScratchPad(addr, data);
    //if scratchPad read failed then continue to next 1-Wire device
    if (!scratchPadReaded)
      continue;

    //if config is not correct
    if (data[2] != 0x50 || data[3] != 0x00 || data[4] != 0x7F)
    {

      //write ScratchPad with Th=80°C, Tl=0°C, Config 12bits resolution
      WriteScratchPad(addr, 0x50, 0x00, 0x7F);

      scratchPadReaded = ReadScratchPad(addr, data);
      //if scratchPad read failed then continue to next 1-Wire device
      if (!scratchPadReaded)
        continue;

      //so we finally can copy scratchpad to memory
      CopyScratchPad(addr);
    }
  }
#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif
}
//------------------------------------------
// DS18X20 Start Temperature conversion
void DS18B20Bus::StartConvertT()
{
#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  reset();
  skip();
  write(0x44); // start conversion

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif
}
//------------------------------------------
// DS18X20 Read Temperatures from all sensors
void DS18B20Bus::ReadTemperatures()
{
  //tempList will receive new list and values
  TemperatureList *tempList = new TemperatureList();

  uint8_t romCode[8];

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //list all romCodes
  reset_search();
  while (search(romCode))
  {

    //if ROM received is incorrect or not a Temperature sensor THEN continue to next device
    if ((crc8(romCode, 7) != romCode[7]) || (romCode[0] != 0x10 && romCode[0] != 0x22 && romCode[0] != 0x28))
      continue;

    //allocate memory
    if (!tempList->romCodes)
      tempList->romCodes = (byte(*)[8])malloc(++tempList->nbSensors * 8 * sizeof(byte));
    else
      tempList->romCodes = (byte(*)[8])realloc(tempList->romCodes, ++tempList->nbSensors * 8 * sizeof(byte));

    //copy the romCode
    for (byte i = 0; i < 8; i++)
    {
      tempList->romCodes[tempList->nbSensors - 1][i] = romCode[i];
    }
  }

  //prepare temperatures list
  tempList->temperatures = (float *)malloc(tempList->nbSensors * sizeof(float));
  byte data[12]; //buffer that receive scratchpad
  //now read all temperatures
  for (byte i = 0; i < tempList->nbSensors; i++)
  {
    //if read of scratchpad failed (3 times inside function) then put NaN in the list
    if (!ReadScratchPad(tempList->romCodes[i], data))
      tempList->temperatures[i] = (0.0 / 0.0); //NaN
    else                                       //else we were able to read data from sensor
    {

      // Convert the data to actual temperature
      // because the result is a 16 bit signed integer, it should
      // be stored to an "int16_t" type, which is always 16 bits
      // even when compiled on a 32 bit processor.
      int16_t raw = (data[1] << 8) | data[0];
      if (tempList->romCodes[i][0] == 0x10)
      {                 //type S temp Sensor
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10)
        {
          // "count remain" gives full 12 bit resolution
          raw = (raw & 0xFFF0) + 12 - data[6];
        }
      }
      else
      {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
          raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
          raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
          raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
      }

      //final temperature is raw/16

      //return temperature
      tempList->temperatures[i] = (float)raw / 16.0;
    }
  }

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

  //Our new value list is ready
  //switch to make it available
  TemperatureList *oldList = temperatureList; //backup current one
  temperatureList = tempList;                 //make new one available

  //now we need to free memory of the old one
  if (oldList)
  {
    if (oldList->romCodes)
      free(oldList->romCodes);
    if (oldList->temperatures)
      free(oldList->temperatures);
    free(oldList);
  }
}
//------------------------------------------
// List DS18X20 sensor ROMCode and return it in JSON list
String DS18B20Bus::GetRomCodeListJSON()
{
  //prepare JSON structure
  String grclJSON(F("{\"TemperatureSensorList\": [\r\n"));

  if (temperatureList)
  {
    for (byte i = 0; i < temperatureList->nbSensors; i++)
    {

      //populate JSON answer with romCode found
      if (i)
        grclJSON += F(",\r\n");

      grclJSON += '"';
      for (byte j = 0; j < 8; j++)
      {
        if (temperatureList->romCodes[i][j] < 16)
          grclJSON += '0';
        grclJSON += String(temperatureList->romCodes[i][j], HEX);
      }
      grclJSON += '"';
    }
  }

  //Finalize JSON structure
  grclJSON += F("\r\n]}");

  return grclJSON;
}

//------------------------------------------
// function that get temperature from a DS18X20 and return it in float (run convertion, get scratchpad then calculate temperature)
float DS18B20Bus::GetTemp(byte addr[] = NULL)
{

  if (!temperatureList)
    return (0.0 / 0.0);

  byte pos = 0;

  while (pos < temperatureList->nbSensors)
  {
    //if romCode match
    if (addr[0] == temperatureList->romCodes[pos][0] && addr[1] == temperatureList->romCodes[pos][1] && addr[2] == temperatureList->romCodes[pos][2] && addr[3] == temperatureList->romCodes[pos][3] && addr[4] == temperatureList->romCodes[pos][4] && addr[5] == temperatureList->romCodes[pos][5] && addr[6] == temperatureList->romCodes[pos][6] && addr[7] == temperatureList->romCodes[pos][7])
      break; //stop while
    pos++;
  }

  if (pos == temperatureList->nbSensors) //if we reached the end of the list
    return (0.0 / 0.0);                  //return NaN
  else
    return temperatureList->temperatures[pos]; //return temperature
}

//------------------------------------------
// function that get temperature from a DS18X20 and return it in JSON
String DS18B20Bus::GetTempJSON(byte addr[])
{
  //get temperature
  float res = GetTemp(addr);

  //if read failed return empty string
  if (std::isnan(res))
    return String();

  //otherwise make and return JSON
  String gtJSON(F("{\"Temperature\": "));
  gtJSON += String(res, 2);
  gtJSON = gtJSON + '}';

  return gtJSON;
}

//------------------------------------------
// function that get temperature from all DS18X20 and return it in JSON
String DS18B20Bus::GetAllTempJSON()
{
  //JSON string to return
  String gatJSON('{');

  if (temperatureList)
  {
    //for each sensors
    for (byte i = 0; i < temperatureList->nbSensors; i++)
    {
      //build JSON
      if (i)
        gatJSON += ',';

      gatJSON += '"';
      for (byte j = 0; j < 8; j++)
      {
        if (temperatureList->romCodes[i][j] < 16)
          gatJSON += '0';
        gatJSON += String(temperatureList->romCodes[i][j], HEX);
      }
      gatJSON += F("\":");
      if (!std::isnan(temperatureList->temperatures[i]))
        gatJSON += String(temperatureList->temperatures[i], 2);
      else
        gatJSON += F("\"NaN\"");
    }
  }

  gatJSON += '}';

  //return JSON temperatures
  return gatJSON;
}

//----------------------------------------------------------------------
// --- WebDS18B20Buses Class---
//----------------------------------------------------------------------
//------------------------------------------
// return True if s contain only hexadecimal figure
boolean WebDS18B20Buses::isROMCodeString(const char *s)
{

  if (strlen(s) != 16)
    return false;
  for (int i = 0; i < 16; i++)
  {
    if (!isHexadecimalDigit(s[i]))
      return false;
  }
  return true;
}

//------------------------------------------
// Execute code to start temperature conversion of all sensors
void WebDS18B20Buses::ConvertTick()
{
  //For all Buses
  for (byte busNumber = 0; busNumber < numberOfBuses; busNumber++)
  {
    _ds18b20Buses[busNumber]->StartConvertT();
  }
  delay(800);
  //For all Buses
  for (byte busNumber = 0; busNumber < numberOfBuses; busNumber++)
  {
    _ds18b20Buses[busNumber]->ReadTemperatures();
  }
}

//------------------------------------------
// Execute code to upload temperature to MQTT if enable
void WebDS18B20Buses::UploadTick()
{
  //if Home Automation upload not enabled then return
  if (ha.protocol == HA_PROTO_DISABLED)
    return;

  //----- MQTT Protocol configured -----
  if (ha.protocol == HA_PROTO_MQTT)
  {
    //sn can be used in multiple cases
    char sn[9];
    sprintf_P(sn, PSTR("%08x"), ESP.getChipId());

    //if not connected to MQTT
    if (!_pubSubClient->connected())
    {
      //generate clientID
      String clientID(F(APPLICATION1_NAME));
      clientID += sn;
      //and try to connect
      if (!ha.mqtt.username[0])
        _pubSubClient->connect(clientID.c_str());
      else
      {
        if (!ha.mqtt.password[0])
          _pubSubClient->connect(clientID.c_str(), ha.mqtt.username, NULL);
        else
          _pubSubClient->connect(clientID.c_str(), ha.mqtt.username, ha.mqtt.password);
      }
    }

    //if still not connected
    if (!_pubSubClient->connected())
    {
      //return error code minus 10 (result should be negative)
      _haSendResult = _pubSubClient->state();
      _haSendResult -= 10;
    }
    // else we are connected
    else
    {
      //prepare topic
      String completeTopic, thisSensorTopic;
      switch (ha.mqtt.type)
      {
      case HA_MQTT_GENERIC_1:
        completeTopic = ha.mqtt.generic.baseTopic;

        //check for final slash
        if (completeTopic.length() && completeTopic.charAt(completeTopic.length()-1) != '/')
          completeTopic += '/';

        //complete the topic
        completeTopic += F("$romcode$/temperature");
        break;
      case HA_MQTT_GENERIC_2:
        completeTopic = ha.mqtt.generic.baseTopic;

        //check for final slash
        if (completeTopic.length() && completeTopic.charAt(completeTopic.length()-1) != '/')
          completeTopic += '/';

        //complete the topic
        completeTopic += F("$romcode$");
        break;
      }

      //Replace placeholders
      if (completeTopic.indexOf(F("$sn$")) != -1)
        completeTopic.replace(F("$sn$"), sn);

      if (completeTopic.indexOf(F("$mac$")) != -1)
        completeTopic.replace(F("$mac$"), WiFi.macAddress());

      if (completeTopic.indexOf(F("$model$")) != -1)
        completeTopic.replace(F("$model$"), APPLICATION1_NAME);

      char romCodeA[17] = {0};
      char busNumberA[2] = {0, 0};

      //For all Buses
      for (byte busNumber = 0; busNumber < numberOfBuses; busNumber++)
      {
        if (_ds18b20Buses[busNumber]->temperatureList) //if there is a list for this bus
        {
          //for each sensors found
          for (byte i = 0; i < _ds18b20Buses[busNumber]->temperatureList->nbSensors; i++)
          {
            //if temperature is OK
            if (!std::isnan(_ds18b20Buses[busNumber]->temperatureList->temperatures[i]))
            {

              //convert romCode to text in oneshot
              sprintf_P(romCodeA, PSTR("%02x%02x%02x%02x%02x%02x%02x%02x"), _ds18b20Buses[busNumber]->temperatureList->romCodes[i][0], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][1], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][2], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][3], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][4], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][5], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][6], _ds18b20Buses[busNumber]->temperatureList->romCodes[i][7]);

              //copy completeTopic in order to "complete" it ...
              thisSensorTopic = completeTopic;

              if (thisSensorTopic.indexOf(F("$romcode$")) != -1)
                thisSensorTopic.replace(F("$romcode$"), romCodeA);

              busNumberA[0] = busNumber + '0';
              if (thisSensorTopic.indexOf(F("$bus$")) != -1)
                thisSensorTopic.replace(F("$bus$"), busNumberA);

              //send
              _haSendResult = _pubSubClient->publish(thisSensorTopic.c_str(), String(_ds18b20Buses[busNumber]->temperatureList->temperatures[i], 2).c_str());
            }
          }
        }
      }
    }
  }
}

//------------------------------------------
//Used to initialize configuration properties to default values
void WebDS18B20Buses::SetConfigDefaultValues()
{
  numberOfBuses = 0;
  memset(owBusesPins, 0, MAX_NUMBER_OF_BUSES * 2);
  for (byte i = 0; i < MAX_NUMBER_OF_BUSES; i++)
    _ds18b20Buses[i] = NULL;

#if ESP01_PLATFORM
  numberOfBuses = 1;
  owBusesPins[0][0] = 3;
  owBusesPins[0][1] = 0;
#endif

#if WFP_DS18B20_PIN
  numberOfBuses = 1;
  owBusesPins[0][0] = WFP_DS18B20_PIN;
#endif

  ha.protocol = HA_PROTO_DISABLED;
  ha.tls = false;
  ha.hostname[0] = 0;
  ha.uploadPeriod = 60;

  ha.mqtt.type = HA_MQTT_GENERIC_1;
  ha.mqtt.port = 1883;
  ha.mqtt.username[0] = 0;
  ha.mqtt.password[0] = 0;
  ha.mqtt.generic.baseTopic[0] = 0;
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebDS18B20Buses::ParseConfigJSON(JsonObject &root)
{
#if ESP01_PLATFORM
  numberOfBuses = 1;
  owBusesPins[0][0] = 3;
  owBusesPins[0][1] = 0;
#else
  // numberOfBuses = root[F("n")];
  // JsonArray &obpArray = root[F("obp")];
  // obpArray.copyTo(owBusesPins);
#endif

#if WFP_DS18B20_PIN
  numberOfBuses = 1;
  owBusesPins[0][0] = WFP_DS18B20_PIN;
#endif

  if (root[F("haproto")].success())
    ha.protocol = root[F("haproto")];
  if (root[F("hatls")].success())
    ha.tls = root[F("hatls")];
  if (root[F("hahost")].success())
    strlcpy(ha.hostname, root[F("hahost")], sizeof(ha.hostname));
  if (root[F("haupperiod")].success())
    ha.uploadPeriod = root[F("haupperiod")];

  if (root[F("hamtype")].success())
    ha.mqtt.type = root[F("hamtype")];
  if (root[F("hamport")].success())
    ha.mqtt.port = root[F("hamport")];
  if (root[F("hamu")].success())
    strlcpy(ha.mqtt.username, root[F("hamu")], sizeof(ha.mqtt.username));
  if (root[F("hamp")].success())
    strlcpy(ha.mqtt.password, root[F("hamp")], sizeof(ha.mqtt.password));

  if (root[F("hamgbt")].success())
    strlcpy(ha.mqtt.generic.baseTopic, root[F("hamgbt")], sizeof(ha.mqtt.generic.baseTopic));
};
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebDS18B20Buses::ParseConfigWebRequest(AsyncWebServerRequest *request)
{

#if ESP01_PLATFORM
  numberOfBuses = 1;
  owBusesPins[0][0] = 3;
  owBusesPins[0][1] = 0;

#else

  // char tempNumberOfBusesA[2]; //only one char
  // if (!request->hasParam(F("n"), true))
  // {
  //   request->send(400, F("text/html"), F("Missing number of OW Buses"));
  //   return false;
  // }

  // numberOfBuses = request->getParam(F("n"), true)->value().toInt();
  // if (numberOfBuses < 1 || numberOfBuses > MAX_NUMBER_OF_BUSES)
  // {
  //   request->send(400, F("text/html"), F("Incorrect number of OW Buses"));
  //   return false;
  // }
  // char busPinName[4] = {'b', '0', 'i', 0};
  // for (byte i = 0; i < numberOfBuses; i++)
  // {
  //   char busPinA[4] = {0};
  //   busPinName[1] = '0' + i;
  //   busPinName[2] = 'i';
  //   if (!request->hasParam(busPinName, true))
  //   {
  //     request->send(400, F("text/html"), F("A PinIn value is missing"));
  //     return false;
  //   }
  //   if (request->getParam(busPinName, true)->value().toInt() == 0 && request->getParam(busPinName, true)->value() != "0")
  //   {
  //     request->send(400, F("text/html"), F("A PinIn value is incorrect"));
  //     return false;
  //   }
  //   owBusesPins[i][0] = request->getParam(busPinName, true)->value().toInt();

  //   busPinA[0] = 0;
  //   busPinName[2] = 'o';
  //   if (!request->hasParam(busPinName, true))
  //   {
  //     request->send(400, F("text/html"), F("A PinOut value is missing"));
  //     return false;
  //   }
  //   if (request->getParam(busPinName, true)->value().toInt() == 0 && request->getParam(busPinName, true)->value() != "0")
  //   {
  //     request->send(400, F("text/html"), F("A PinOut value is incorrect"));
  //     return false;
  //   }
  //   owBusesPins[i][1] = request->getParam(busPinName, true)->value().toInt();
  // }
#endif

#if WFP_DS18B20_PIN
  numberOfBuses = 1;
  owBusesPins[0][0] = WFP_DS18B20_PIN;
#endif

  //Parse HA protocol
  if (request->hasParam(F("haproto"), true))
    ha.protocol = request->getParam(F("haproto"), true)->value().toInt();

  //if an home Automation protocol has been selected then get common param
  if (ha.protocol != HA_PROTO_DISABLED)
  {
    if (request->hasParam(F("hatls"), true))
      ha.tls = (request->getParam(F("hatls"), true)->value() == F("on"));
    else
      ha.tls = false;
    if (request->hasParam(F("hahost"), true) && request->getParam(F("hahost"), true)->value().length() < sizeof(ha.hostname))
      strcpy(ha.hostname, request->getParam(F("hahost"), true)->value().c_str());
    if (request->hasParam(F("haupperiod"), true))
      ha.uploadPeriod = request->getParam(F("haupperiod"), true)->value().toInt();
  }

  //Now get specific param
  switch (ha.protocol)
  {

  case HA_PROTO_MQTT:

    if (request->hasParam(F("hamtype"), true))
      ha.mqtt.type = request->getParam(F("hamtype"), true)->value().toInt();
    if (request->hasParam(F("hamport"), true))
      ha.mqtt.port = request->getParam(F("hamport"), true)->value().toInt();
    if (request->hasParam(F("hamu"), true) && request->getParam(F("hamu"), true)->value().length() < sizeof(ha.mqtt.username))
      strcpy(ha.mqtt.username, request->getParam(F("hamu"), true)->value().c_str());
    char tempPassword[64 + 1] = {0};
    //put MQTT password into temporary one for predefpassword
    if (request->hasParam(F("hamp"), true) && request->getParam(F("hamp"), true)->value().length() < sizeof(tempPassword))
      strcpy(tempPassword, request->getParam(F("hamp"), true)->value().c_str());
    //check for previous password (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempPassword, appDataPredefPassword))
      strcpy(ha.mqtt.password, tempPassword);

    switch (ha.mqtt.type)
    {
    case HA_MQTT_GENERIC_1:
    case HA_MQTT_GENERIC_2:
      if (request->hasParam(F("hamgbt"), true) && request->getParam(F("hamgbt"), true)->value().length() < sizeof(ha.mqtt.generic.baseTopic))
        strcpy(ha.mqtt.generic.baseTopic, request->getParam(F("hamgbt"), true)->value().c_str());

      if (!ha.hostname[0] || !ha.mqtt.generic.baseTopic[0])
        ha.protocol = HA_PROTO_DISABLED;
      break;
    }
    break;
  }

  return true;
};
//------------------------------------------
//Generate JSON from configuration properties
String WebDS18B20Buses::GenerateConfigJSON(bool forSaveFile = false)
{
  String gc('{');

#if ESP01_PLATFORM
  if (!forSaveFile)
  {
    gc += F("\"e\":1,\"n\":1,\"nm\":1,\"b0i\":3,\"b0o\":0");
  }
  else
  {
    gc += F("\"n\":1,\"obp\":[[3,0]]");
  }

#else
  // gc = gc + F("\"n\":") + numberOfBuses;

  // if (!forSaveFile)
  // {
  //   gc = gc + F(",\"nm\":") + MAX_NUMBER_OF_BUSES;
  //   for (byte i = 0; i < numberOfBuses; i++)
  //   {
  //     gc = gc + F(",\"b") + i + F("i\":") + owBusesPins[i][0];
  //     gc = gc + F(",\"b") + i + F("o\":") + owBusesPins[i][1];
  //   }
  // }
  // else
  // {
  //   gc = gc + F(",\"obp\":[");
  //   for (byte i = 0; i < numberOfBuses; i++)
  //   {
  //     if (i)
  //       gc += ',';
  //     gc = gc + '[' + owBusesPins[i][0] + ',' + owBusesPins[i][1] + ']';
  //   }
  //   gc += ']';
  // }
#endif

#if WFP_DS18B20_PIN
  if (!forSaveFile)
  {
    gc += F("\"e\":1,\"n\":1,\"nm\":1,\"b0i\":13,\"b0o\":0");
  }
  else
  {
    gc += F("\"n\":1,\"obp\":[[13,0]]");
  }
#endif

  gc = gc + F(",\"haproto\":") + ha.protocol;
  gc = gc + F(",\"hatls\":") + ha.tls;
  gc = gc + F(",\"hahost\":\"") + ha.hostname + '"';
  gc = gc + F(",\"haupperiod\":") + ha.uploadPeriod;

  //if for WebPage or protocol selected is MQTT
  if (!forSaveFile || ha.protocol == HA_PROTO_MQTT)
  {
    gc = gc + F(",\"hamtype\":") + ha.mqtt.type;
    gc = gc + F(",\"hamport\":") + ha.mqtt.port;
    gc = gc + F(",\"hamu\":\"") + ha.mqtt.username + '"';
    if (forSaveFile)
      gc = gc + F(",\"hamp\":\"") + ha.mqtt.password + '"';
    else
      gc = gc + F(",\"hamp\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)

    gc = gc + F(",\"hamgbt\":\"") + ha.mqtt.generic.baseTopic + '"';
  }

  gc += '}';

  return gc;
};
//------------------------------------------
//Generate JSON of application status
String WebDS18B20Buses::GenerateStatusJSON()
{
  String gs('{');

  for (byte i = 0; i < numberOfBuses; i++)
  {
    gs = gs + (i ? "," : "") + '"' + i + F("\":");

    gs = gs + _ds18b20Buses[i]->GetAllTempJSON();
  }

  if (ha.protocol != HA_PROTO_DISABLED)
    gs = gs + F(",\"lhar\":") + _haSendResult;
  else
    gs = gs + F(",\"lhar\":\"NA\"");

  gs = gs + '}';

  return gs;
};
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebDS18B20Buses::AppInit(bool reInit)
{

  //Clean up MQTT variables
  if (_pubSubClient)
  {
    if (_pubSubClient->connected())
      _pubSubClient->disconnect();
    delete _pubSubClient;
    _pubSubClient = NULL;
  }
  if (_wifiClient)
  {
    delete _wifiClient;
    _wifiClient = NULL;
  }
  if (_wifiClientSecure)
  {
    delete _wifiClientSecure;
    _wifiClientSecure = NULL;
  }

  //if MQTT used so build MQTT variables
  if (ha.protocol == HA_PROTO_MQTT)
  {

    if (!ha.tls)
    {
      _wifiClient = new WiFiClient();
      _pubSubClient = new PubSubClient(ha.hostname, ha.mqtt.port, *_wifiClient);
    }
    else
    {
      _wifiClientSecure = new WiFiClientSecure();
      _pubSubClient = new PubSubClient(ha.hostname, ha.mqtt.port, *_wifiClientSecure);
    }
  }

  //cleanup DS18B20Buses
  for (byte i = 0; i < MAX_NUMBER_OF_BUSES; i++)
  {

    if (_ds18b20Buses[i])
    {
      if (_ds18b20Buses[i]->temperatureList)
      {
        if (_ds18b20Buses[i]->temperatureList->romCodes)
          free(_ds18b20Buses[i]->temperatureList->romCodes);
        if (_ds18b20Buses[i]->temperatureList->temperatures)
          free(_ds18b20Buses[i]->temperatureList->temperatures);
        free(_ds18b20Buses[i]->temperatureList);
      }

      delete _ds18b20Buses[i];
      _ds18b20Buses[i] = NULL;
    }
  }

  _initialized = numberOfBuses > 0;

//Special case because of DS18B20Bus constructor
#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //create DS18B20 objects
  for (byte i = 0; i < numberOfBuses; i++)
  {
#if WFP_DS18B20_PIN
    _ds18b20Buses[i] = new DS18B20Bus(owBusesPins[i][0]);
#endif
    _ds18b20Buses[i]->SetupTempSensors();
  }

//Special case because of DS18B20Bus constructor
#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

  //cleanup Timers
  if (_timers[1].getNumTimers()) //HA Timer
    _timers[1].deleteTimer(0);
  if (_timers[0].getNumTimers()) //temperature Refresh Timer
    _timers[0].deleteTimer(0);

  //reset _haSendResult
  _haSendResult = 0;

  //if no HA, then use default period
  if (ha.protocol == HA_PROTO_DISABLED)
  {
    //setup temperature conversion
    _timers[0].setInterval(1000L * DEFAULT_CONVERT_PERIOD, [this]() { this->ConvertTick(); });
  }
  else
  {
    _timers[0].setInterval(1000L * ha.uploadPeriod, [this]() { this->ConvertTick(); });
    _timers[1].setInterval(1000L * ha.uploadPeriod, [this]() { this->UploadTick(); });
  }

  return true;
};
//------------------------------------------
//Return HTML Code to insert into Status Web page
const uint8_t* WebDS18B20Buses::GetHTMLContent(WebPageForPlaceHolder wp){
      switch(wp){
    case status:
      return (const uint8_t*) status2htmlgz;
      break;
    case config:
      return (const uint8_t*) config2htmlgz;
      break;
    default:
      return nullptr;
      break;
  };
  return nullptr;
};
//and his Size
size_t WebDS18B20Buses::GetHTMLContentSize(WebPageForPlaceHolder wp){
  switch(wp){
    case status:
      return sizeof(status2htmlgz);
      break;
    case config:
      return sizeof(config2htmlgz);
      break;
    default:
      return 0;
      break;
  };
  return 0;
};
//------------------------------------------
//code to register web request answer to the web server
void WebDS18B20Buses::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{
  server.on("/getL", HTTP_GET, [this](AsyncWebServerRequest *request) {

    bool requestPassed = false;
    byte busNumberPassed = 0;

    //check DS18B20Buses is initialized
    if (!_initialized)
    {
      request->send(400, F("text/html"), F("Buses not Initialized"));
      return;
    }

    char paramName[5] = {'b', 'u', 's', '0', 0};
    byte i = 0;

    //for each bus numbers
    while (i < numberOfBuses && !requestPassed)
    {
      //build paramName
      paramName[3] = '0' + i;

      //check bus param is there
      if (request->hasParam(paramName))
      {
        busNumberPassed = i;
        requestPassed = true;
      }
      i++;
    }

    //if no correct request passed
    if (!requestPassed)
    {
      //answer with error and return
      request->send(400, F("text/html"), F("No valid request received"));
      return;
    }

    //list OneWire Temperature sensors
    request->send(200, F("text/json"), _ds18b20Buses[busNumberPassed]->GetRomCodeListJSON());
  });

  server.on("/getT", HTTP_GET, [this](AsyncWebServerRequest *request) {

    bool requestPassed = false;
    byte busNumberPassed = 0;
    byte romCodePassed[8];

    //check DS18B20Buses is initialized
    if (!_initialized)
    {
      request->send(400, F("text/html"), F("Buses not Initialized"));
      return;
    }

    char paramName[5] = {'b', 'u', 's', '0', 0};
    byte i = 0;

    //for each bus numbers
    while (i < numberOfBuses && !requestPassed)
    {
      //build paramName
      paramName[3] = '0' + i;

      //check bus param is there
      if (request->hasParam(paramName))
      {

        //get ROMCode
        const char *ROMCodeA = request->getParam(paramName)->value().c_str();
        //if it's a correct ROMCode
        if (isROMCodeString(ROMCodeA))
        {
          //Parse it
          for (byte j = 0; j < 8; j++)
            romCodePassed[j] = (Utils::AsciiToHex(ROMCodeA[j * 2]) * 0x10) + Utils::AsciiToHex(ROMCodeA[(j * 2) + 1]);
          busNumberPassed = i;
          requestPassed = true;
        }
      }
      i++;
    }

    //if no correct request passed
    if (!requestPassed)
    {
      //answer with error and return
      request->send(400, F("text/html"), F("No valid request received"));
      return;
    }

    //Read Temperature
    String temperatureJSON = _ds18b20Buses[busNumberPassed]->GetTempJSON(romCodePassed);

    if (temperatureJSON.length() > 0)
      request->send(200, F("text/json"), temperatureJSON);
    else
      request->send(500, F("text/html"), F("Read sensor failed"));
  });
};

//------------------------------------------
//Run for timer
void WebDS18B20Buses::AppRun()
{
  if (_pubSubClient)
    _pubSubClient->loop();
  if (_timers[0].getNumTimers())
    _timers[0].run();
  if (_timers[1].getNumTimers())
    _timers[1].run();
}

//------------------------------------------
//Constructor
WebDS18B20Buses::WebDS18B20Buses(char appId, String appName) : Application(appId, appName)
{
  //Nothing to do
}