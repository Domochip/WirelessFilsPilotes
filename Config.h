#ifndef Config_h
#define Config_h


#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "data\config.html.gz.h"

#include "WirelessFilsPilotes.h"

#define DEFAULT_AP_SSID "WirelessFilsPilotes"
#define DEFAULT_AP_PSK "PasswordFP"

const char predefPassword[] PROGMEM = "ewcXoCt4HHjZUvY0";

class Config {
  public:
    char ssid[32 + 1] = {0};
    char password[64 + 1] = {0};
    char hostname[24 + 1] = {0};
    
    void SetDefaultValues() {
      ssid[0] = 0;
      password[0] = 0;
      hostname[0] = 0;
    }

    static byte AsciiToHex(char c); //Utils
    static bool FingerPrintS2A(byte* fingerPrintArray, const char* fingerPrintToDecode);
    static char* FingerPrintA2S(char* fpBuffer, byte* fingerPrintArray, char separator = 0);

    bool Save();
    bool Load();
    void InitWebServer(AsyncWebServer &server, bool &shouldReboot);
  private :
    String GetJSON();
    bool SetFromParameters(AsyncWebServerRequest* request);
    uint16_t crc; ///!\ crc should always stay in last position
};


#endif

