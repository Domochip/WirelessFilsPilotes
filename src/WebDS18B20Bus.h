#ifndef WebDS18B20Bus_h
#define WebDS18B20Bus_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "base\Utils.h"
#include "base\Application.h"

const char appDataPredefPassword2[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status2.html.gz.h"
#include "data\config2.html.gz.h"

#include <math.h> //for std::isnan
#include <OneWire.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define DEFAULT_CONVERT_PERIOD 30 //Period in seconds used to refresh sensor tmperature if no MQTT used

#define WFP_DS18B20_PIN 13

//intermediate class that corresponds to a OneWire Bus with DS18B20 sensors
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
class WebDS18B20Bus : public Application
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

  DS18B20Bus *_ds18b20Bus;

  HomeAutomation _ha;
  int _haSendResult = 0;

  bool _owInitialized = false;
  bool _needConvert = false;
  Ticker _convertTicker;
  bool _needPublish = false;
  Ticker _publishTicker;
  WiFiClient _wifiMqttClient;
  WiFiClientSecure _wifiMqttClientSecure;
  PubSubClient _mqttClient;
  bool _needMqttReconnect = false;
  Ticker _mqttReconnectTicker;

  boolean isROMCodeString(const char *s);

  void ConvertTick();
  bool MqttConnect();
  void MqttCallback(char *topic, uint8_t *payload, unsigned int length);
  void PublishTick();

  void SetConfigDefaultValues();
  void ParseConfigJSON(DynamicJsonDocument &doc);
  bool ParseConfigWebRequest(AsyncWebServerRequest *request);
  String GenerateConfigJSON(bool forSaveFile);
  String GenerateStatusJSON();
  bool AppInit(bool reInit);
  const uint8_t *GetHTMLContent(WebPageForPlaceHolder wp);
  size_t GetHTMLContentSize(WebPageForPlaceHolder wp);
  void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void AppRun();

public:
  WebDS18B20Bus(char appId, String fileName);
};

#endif