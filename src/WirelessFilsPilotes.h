#ifndef WirelessFilsPilotes_h
#define WirelessFilsPilotes_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "base\Utils.h"
#include "base\Base.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status1.html.gz.h"
#include "data\config1.html.gz.h"

#include <ESP8266HTTPClient.h>
#include <Adafruit_MCP23017.h>
#include "SimpleTimer.h"

class WebFP : public Application
{

  private:
    //FP Names/Aliases
    char fpNames[8][25];

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
    SimpleTimer _comfortTimer[8]; //8 SimpleTimer Object with max 2 timers inside (SimpleTimer.h)
    byte _fpStates[8] = {51, 51, 51, 51, 51, 51, 51, 51};

    void TimerTickON(byte fpNumber, byte liveOnDuration);
    void TimerTickOFF(byte fpNumber);

    void setFP(byte fpNumber, byte stateNumber, bool force = false);

    void SetConfigDefaultValues();
    void ParseConfigJSON(JsonObject &root);
    bool ParseConfigWebRequest(AsyncWebServerRequest *request);
    String GenerateConfigJSON(bool forSaveFile);
    String GenerateStatusJSON();
    bool AppInit(bool reInit);
    const uint8_t *GetHTMLContent(WebPageForPlaceHolder wp);
    size_t GetHTMLContentSize(WebPageForPlaceHolder wp);
    void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
    void AppRun();

  public:
    WebFP(char appId, String fileName);
};


#include "data\status2.html.gz.h"
#include "data\config2.html.gz.h"

#include <math.h> //for std::isnan
#include <OneWire.h>
#include <PubSubClient.h>
#include "SimpleTimer.h"

#define MAX_NUMBER_OF_BUSES 1
#define DEFAULT_CONVERT_PERIOD 30 //Period in seconds used to refresh sensor tmperature if no MQTT used

#define WFP_DS18B20_PIN 13

//intermediate class that corresponds to a OneWire Bus with DS12B20 sensors
class DS18B20Bus : public OneWire
{
private:
  boolean ReadScratchPad(byte addr[], byte data[]);
  void WriteScratchPad(byte addr[], byte th, byte tl, byte cfg);
  void CopyScratchPad(byte addr[]);

public:
  typedef struct
  {
    uint8_t nbSensors = 0;
    byte (*romCodes)[8] = NULL;
    float *temperatures = NULL;
  } TemperatureList;

  TemperatureList *temperatureList = NULL;

  DS18B20Bus(uint8_t pin);
  void SetupTempSensors(); //Set sensor to 12bits resolution
  void StartConvertT();
  void ReadTemperatures();

  String GetRomCodeListJSON();
  float GetTemp(byte addr[]);
  String GetTempJSON(byte addr[]);
  byte GetAllTemp(uint8_t *&romCodes, float *&temperatures);
  String GetAllTempJSON();
};

//Real Application class
class WebDS18B20Buses : public Application
{
private:
#define HA_MQTT_GENERIC_1 0 //separated level topic (/$romcode$/temperature)
#define HA_MQTT_GENERIC_2 1 //same level topic (/$romcode$)

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
    bool tls = false;
    char hostname[64 + 1] = {0};
    uint16_t uploadPeriod = 60;
    MQTT mqtt;
  } HomeAutomation;

  byte numberOfBuses = 0;
  uint8_t owBusesPins[MAX_NUMBER_OF_BUSES][2];

  DS18B20Bus *_ds18b20Buses[MAX_NUMBER_OF_BUSES];

  HomeAutomation ha;
  int _haSendResult = 0;

  bool _initialized = false;
  SimpleTimer _timers[2]; //(0: for Temperature conversion; 1: for HA if enabled)
  WiFiClient *_wifiClient = NULL;
  WiFiClientSecure *_wifiClientSecure = NULL;
  PubSubClient *_pubSubClient = NULL;

  boolean isROMCodeString(const char *s);

  void ConvertTick();
  void UploadTick();

  void SetConfigDefaultValues();
  void ParseConfigJSON(JsonObject &root);
  bool ParseConfigWebRequest(AsyncWebServerRequest *request);
  String GenerateConfigJSON(bool forSaveFile);
  String GenerateStatusJSON();
  bool AppInit(bool reInit);
  const uint8_t* GetHTMLContent(WebPageForPlaceHolder wp);
  size_t GetHTMLContentSize(WebPageForPlaceHolder wp);
  void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void AppRun();

public:
  WebDS18B20Buses(char appId, String fileName);
};

#endif