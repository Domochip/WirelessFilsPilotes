#ifndef WirelessFilsPilotes_h
#define WirelessFilsPilotes_h

#include "Main.h"
#include "base\Utils.h"
#include "base\MQTTMan.h"
#include "base\Application.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status1.html.gz.h"
#include "data\config1.html.gz.h"

#include <ESP8266HTTPClient.h>
#include <Adafruit_MCP23017.h>
#include <Ticker.h>

class WebFP : public Application
{

private:
#define HA_MQTT_GENERIC_1 0 //FP separated (/1/command; /1/order; ... /2/command: /2/order; ...)
#define HA_MQTT_GENERIC_2 1 //All FP in the same topic (/command1; /command2; ... /order1; /order2; ...)

  //FP Names/Aliases
  char _fpNames[8][25];

  typedef struct
  {
    byte type = HA_MQTT_GENERIC_1;
    uint32_t port = 1883;
    char username[128 + 1] = {0};
    char password[150 + 1] = {0};
    struct
    {
      char baseTopic[64 + 1] = {0};
    } generic;
  } MQTT;

#define HA_PROTO_DISABLED 0
#define HA_PROTO_MQTT 1

  typedef struct
  {
    byte protocol = HA_PROTO_DISABLED;
    char hostname[64 + 1] = {0};
    MQTT mqtt;
  } HomeAutomation;

  HomeAutomation _ha;
  int _haSendResult = 0;
  WiFiClient _wifiClient;

  MQTTMan _mqttMan;

  //Pin Map is list of pins by pair corresponding to FilsPilotes
  //{Positive of FP1,Negative of FP1,Positive of FP2,Negative of FP2,etc.,...}
#if (MODEL_WFP > 1)
  const byte _FPPINMAP[16] = {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8};
#else
  const byte _FPPINMAP[16] = {5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
#if (MODEL_WFP > 1)
  Adafruit_MCP23017 _mcp23017;
#endif
  Ticker _comfortTickers[16];
  byte _fpStates[8] = {51, 51, 51, 51, 51, 51, 51, 51};

  void timerTickON(byte fpNumber, byte liveOnDuration);
  void timerTickOFF(byte fpNumber);

  void setFP(byte fpNumber, byte stateNumber, bool force = false);
  void mqttConnectedCallback(MQTTMan *mqttMan, bool firstConnection);
  void mqttCallback(char *topic, uint8_t *payload, unsigned int length);

  void setConfigDefaultValues();
  void parseConfigJSON(DynamicJsonDocument &doc);
  bool parseConfigWebRequest(AsyncWebServerRequest *request);
  String generateConfigJSON(bool forSaveFile);
  String generateStatusJSON();
  bool appInit(bool reInit);
  const uint8_t *getHTMLContent(WebPageForPlaceHolder wp);
  size_t getHTMLContentSize(WebPageForPlaceHolder wp);
  void appInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void appRun();

public:
  WebFP(char appId, String fileName);
};

#endif