#include "WirelessFilsPilotes.h"

//-----------------------------------------------------------------------
// Timer Tick ON (Used to send Live to FP every 5 minutes)
//-----------------------------------------------------------------------
void WebFP::timerTickON(byte fpNumber, byte liveOnDuration)
{
#if DEVELOPPER_MODE
  LOG_SERIAL.println(String(millis() / 1000) + F(" timerTickON : fp=") + fpNumber + F(";duration=") + liveOnDuration);
#endif

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

  //then create Ticker for Stop
  _comfortTickers[fpNumber * 2 + 1].once(liveOnDuration, [this, fpNumber]() {
    this->timerTickOFF(fpNumber);
  });
}

//-----------------------------------------------------------------------
// Timer Tick OFF (Used to stop Live to FP after 3 or 7 sec)
//-----------------------------------------------------------------------
void WebFP::timerTickOFF(byte fpNumber)
{
#if DEVELOPPER_MODE
  LOG_SERIAL.println(String(millis() / 1000) + F(" timerTickOFF : fp=") + fpNumber);
#endif

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
#if DEVELOPPER_MODE
  LOG_SERIAL.println(String(millis() / 1000) + F(" setFP : fp=") + fpNumber + F(";state=") + stateNumber + F(";force=") + force);
#endif

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

      //clean up comfortTicker of current FP (Will be reattached if necessary)
      if (_comfortTickers[fpNumber * 2].active())
        _comfortTickers[fpNumber * 2].detach();
      if (_comfortTickers[fpNumber * 2 + 1].active())
        _comfortTickers[fpNumber * 2 + 1].detach();

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

        //Setup Ticker system
        _comfortTickers[fpNumber * 2].attach(300.0F, [this, fpNumber]() {
          this->timerTickON(fpNumber, 7);
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

        //Setup Ticker system
        _comfortTickers[fpNumber * 2].attach(300.0F, [this, fpNumber]() {
          this->timerTickON(fpNumber, 3);
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

    //need to publish new State to Home Automation
    //----- MQTT Protocol configured -----
    if (_ha.protocol == HA_PROTO_MQTT)
    {
      //if we are connected
      if (_mqttMan.connected())
      {
        //prepare topic
        String completeTopic = _ha.mqtt.generic.baseTopic;

        //Replace placeholders
        MQTTMan::prepareTopic(completeTopic);

        switch (_ha.mqtt.type)
        {
        case HA_MQTT_GENERIC_1:
          //complete the topic
          completeTopic += F("$fpn$/order");
          break;
        case HA_MQTT_GENERIC_2:
          //complete the topic
          completeTopic += F("order$fpn$");
          break;
        }

        //now place the Fil Pilote number
        if (completeTopic.indexOf(F("$fpn$")) != -1)
          completeTopic.replace(F("$fpn$"), String(fpNumber + 1));

        //Then publish
        _haSendResult = _mqttMan.publish(completeTopic.c_str(), String(stateNumber).c_str());
      }
    }

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
// Connect then Subscribe to MQTT
void WebFP::mqttConnectedCallback(MQTTMan *mqttMan, bool firstConnection)
{
  String completeTopic = _ha.mqtt.generic.baseTopic;

  //Replace placeholders
  MQTTMan::prepareTopic(completeTopic);

  //build correct topic
  switch (_ha.mqtt.type)
  {
  case HA_MQTT_GENERIC_1:
    completeTopic += F("$fpn$/command");
    break;
  case HA_MQTT_GENERIC_2:
    completeTopic += F("command$fpn$");
    break;
  }

  //subscribe for each FP
  for (byte i = 0; i < MODEL_WFP; i++)
  {
    String thisTopic = completeTopic;

    if (thisTopic.indexOf(F("$fpn$")) != -1)
      thisTopic.replace(F("$fpn$"), String(i + 1));

    mqttMan->subscribe(thisTopic.c_str());
  }
}
//------------------------------------------
//Callback used when an MQTT message arrived
void WebFP::mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  byte fpValue = 0;

  //check payload length
  if (length < 1 || length > 2)
    return;

  //check payload content
  if (payload[0] < '0' || payload[0] > '9')
    return;
  else
    fpValue = payload[0] - '0';
  if (length == 2)
  {
    if (payload[1] < '0' || payload[1] > '9')
      return;
    else
      fpValue = fpValue * 10 + payload[1] - '0';
  }

  String topicString(topic);

  //for each FP
  for (byte i = 0; i < MODEL_WFP; i++)
  {
    //if the topic match this FilPilote number
    if ((_ha.mqtt.type == HA_MQTT_GENERIC_1 && topicString.endsWith(String('/') + (i + 1) + F("/command"))) || (_ha.mqtt.type == HA_MQTT_GENERIC_2 && topicString.endsWith(String(F("/command")) + (i + 1))))
    {
      //then set it (setFP publish back to order received)
      setFP(i, fpValue);
    }
  }
}
//------------------------------------------
//Used to initialize configuration properties to default values
void WebFP::setConfigDefaultValues()
{
  _fpNames[0][0] = 0;
  _fpNames[1][0] = 0;
  _fpNames[2][0] = 0;
  _fpNames[3][0] = 0;
  _fpNames[4][0] = 0;
  _fpNames[5][0] = 0;
  _fpNames[6][0] = 0;
  _fpNames[7][0] = 0;

  _ha.protocol = HA_PROTO_DISABLED;
  _ha.hostname[0] = 0;

  _ha.mqtt.type = HA_MQTT_GENERIC_1;
  _ha.mqtt.port = 1883;
  _ha.mqtt.username[0] = 0;
  _ha.mqtt.password[0] = 0;
  _ha.mqtt.generic.baseTopic[0] = 0;
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebFP::parseConfigJSON(DynamicJsonDocument &doc)
{
  char fpn[5] = {'f', 'p', '1', 'n', 0};
  for (byte i = 0; i < MODEL_WFP; i++)
  {
    fpn[2] = i + '1';
    if (!doc[fpn].isNull())
      strlcpy(_fpNames[i], doc[fpn], sizeof(_fpNames[i]));
  }

  //Parse Home Automation config
  if (!doc[F("haproto")].isNull())
    _ha.protocol = doc[F("haproto")];
  if (!doc[F("hahost")].isNull())
    strlcpy(_ha.hostname, doc["hahost"], sizeof(_ha.hostname));

  if (!doc[F("hamtype")].isNull())
    _ha.mqtt.type = doc[F("hamtype")];
  if (!doc[F("hamport")].isNull())
    _ha.mqtt.port = doc[F("hamport")];
  if (!doc[F("hamu")].isNull())
    strlcpy(_ha.mqtt.username, doc[F("hamu")], sizeof(_ha.mqtt.username));
  if (!doc[F("hamp")].isNull())
    strlcpy(_ha.mqtt.password, doc[F("hamp")], sizeof(_ha.mqtt.password));

  if (!doc[F("hamgbt")].isNull())
    strlcpy(_ha.mqtt.generic.baseTopic, doc[F("hamgbt")], sizeof(_ha.mqtt.generic.baseTopic));
};
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebFP::parseConfigWebRequest(AsyncWebServerRequest *request)
{
  char fpn[5] = {'f', 'p', '1', 'n', 0};
  for (byte i = 0; i < MODEL_WFP; i++)
  {
    fpn[2] = i + '1';
    if (request->hasParam(fpn, true) && request->getParam(fpn, true)->value().length() < sizeof(_fpNames[i]))
      strcpy(_fpNames[i], request->getParam(fpn, true)->value().c_str());
    else
      _fpNames[i][0] = 0;
  }

  //Parse HA protocol
  if (request->hasParam(F("haproto"), true))
    _ha.protocol = request->getParam(F("haproto"), true)->value().toInt();
  //if an home Automation protocol has been selected then get common param
  if (_ha.protocol != HA_PROTO_DISABLED)
  {
    if (request->hasParam(F("hahost"), true) && request->getParam(F("hahost"), true)->value().length() < sizeof(_ha.hostname))
      strcpy(_ha.hostname, request->getParam(F("hahost"), true)->value().c_str());
  }

  //Now get specific param
  switch (_ha.protocol)
  {
  case HA_PROTO_MQTT:

    if (request->hasParam(F("hamtype"), true))
      _ha.mqtt.type = request->getParam(F("hamtype"), true)->value().toInt();
    if (request->hasParam(F("hamport"), true))
      _ha.mqtt.port = request->getParam(F("hamport"), true)->value().toInt();
    if (request->hasParam(F("hamu"), true) && request->getParam(F("hamu"), true)->value().length() < sizeof(_ha.mqtt.username))
      strcpy(_ha.mqtt.username, request->getParam(F("hamu"), true)->value().c_str());
    char tempPassword[64 + 1] = {0};
    //put MQTT password into temporary one for predefpassword
    if (request->hasParam(F("hamp"), true) && request->getParam(F("hamp"), true)->value().length() < sizeof(tempPassword))
      strcpy(tempPassword, request->getParam(F("hamp"), true)->value().c_str());
    //check for previous password (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempPassword, appDataPredefPassword))
      strcpy(_ha.mqtt.password, tempPassword);

    switch (_ha.mqtt.type)
    {
    case HA_MQTT_GENERIC_1:
    case HA_MQTT_GENERIC_2:
      if (request->hasParam(F("hamgbt"), true) && request->getParam(F("hamgbt"), true)->value().length() < sizeof(_ha.mqtt.generic.baseTopic))
        strcpy(_ha.mqtt.generic.baseTopic, request->getParam(F("hamgbt"), true)->value().c_str());

      if (!_ha.hostname[0] || !_ha.mqtt.generic.baseTopic[0])
        _ha.protocol = HA_PROTO_DISABLED;
      break;
    }
    break;
  }

  return true;
};
//------------------------------------------
//Generate JSON from configuration properties
String WebFP::generateConfigJSON(bool forSaveFile = false)
{
  String gc('{');

  for (byte i = 0; i < MODEL_WFP; i++)
    gc = gc + (i ? "," : "") + F("\"fp") + (i + 1) + F("n\":\"") + _fpNames[i] + '"';

  //Generate Home Automation config information
  gc = gc + F(",\"haproto\":") + _ha.protocol;
  gc = gc + F(",\"hahost\":\"") + _ha.hostname + '"';

  //if for WebPage or protocol selected is MQTT
  if (!forSaveFile || _ha.protocol == HA_PROTO_MQTT)
  {
    gc = gc + F(",\"hamtype\":") + _ha.mqtt.type;
    gc = gc + F(",\"hamport\":") + _ha.mqtt.port;
    gc = gc + F(",\"hamu\":\"") + _ha.mqtt.username + '"';
    if (forSaveFile)
      gc = gc + F(",\"hamp\":\"") + _ha.mqtt.password + '"';
    else
      gc = gc + F(",\"hamp\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)

    gc = gc + F(",\"hamgbt\":\"") + _ha.mqtt.generic.baseTopic + '"';
  }

  gc += '}';

  return gc;
};
//------------------------------------------
//Generate JSON of application status
String WebFP::generateStatusJSON()
{
  String gs('{');

  for (byte i = 0; i < MODEL_WFP; i++)
    gs = gs + (i ? "," : "") + F("\"FP") + (i + 1) + F("\":") + _fpStates[i];

  gs = gs + F(",\"has1\":\"");
  switch (_ha.protocol)
  {
  case HA_PROTO_DISABLED:
    gs = gs + F("Disabled");
    break;
  case HA_PROTO_MQTT:
    gs = gs + F("MQTT Connection State : ");
    switch (_mqttMan.state())
    {
    case MQTT_CONNECTION_TIMEOUT:
      gs = gs + F("Timed Out");
      break;
    case MQTT_CONNECTION_LOST:
      gs = gs + F("Lost");
      break;
    case MQTT_CONNECT_FAILED:
      gs = gs + F("Failed");
      break;
    case MQTT_CONNECTED:
      gs = gs + F("Connected");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      gs = gs + F("Bad Protocol Version");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      gs = gs + F("Incorrect ClientID ");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      gs = gs + F("Server Unavailable");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      gs = gs + F("Bad Credentials");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      gs = gs + F("Connection Unauthorized");
      break;
    }

    if (_mqttMan.state() == MQTT_CONNECTED)
      gs = gs + F("\",\"has2\":\"Last Publish Result : ") + (_haSendResult ? F("OK") : F("Failed"));

    break;
  }
  gs += '"';
  gs += '}';

  return gs;
};
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebFP::appInit(bool reInit)
{
  //Stop MQTT
  _mqttMan.disconnect();

  //if MQTT used so configure it
  if (_ha.protocol == HA_PROTO_MQTT)
  {
    //prepare will topic
    String willTopic = _ha.mqtt.generic.baseTopic;
    MQTTMan::prepareTopic(willTopic);
    willTopic += F("connected");

    //setup MQTT
    _mqttMan.setClient(_wifiClient).setServer(_ha.hostname, _ha.mqtt.port);
    _mqttMan.setConnectedAndWillTopic(willTopic.c_str());
    _mqttMan.setConnectedCallback(std::bind(&WebFP::mqttConnectedCallback, this, std::placeholders::_1, std::placeholders::_2));
    _mqttMan.setCallback(std::bind(&WebFP::mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //Connect
    _mqttMan.connect(_ha.mqtt.username, _ha.mqtt.password);
  }

  if (!reInit)
  {
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
  }

  return true;
};
//------------------------------------------
//Return HTML Code to insert into Status Web page
const uint8_t *WebFP::getHTMLContent(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return (const uint8_t *)status1htmlgz;
    break;
  case config:
    return (const uint8_t *)config1htmlgz;
    break;
  default:
    return nullptr;
    break;
  };
  return nullptr;
};
//and his Size
size_t WebFP::getHTMLContentSize(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
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
void WebFP::appInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{
  server.on("/getFP", HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, F("text/json"), generateStatusJSON());
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
  });

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

    //if no fp order passedm
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
void WebFP::appRun()
{
  if (_ha.protocol == HA_PROTO_MQTT)
    _mqttMan.loop();
}

//------------------------------------------
//Constructor
WebFP::WebFP(char appId, String appName) : Application(appId, appName) {}