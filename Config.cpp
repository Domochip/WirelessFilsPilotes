#include <Arduino.h>
#include <EEPROM.h>

#include "Config.h"

uint16_t crc16(const uint8_t* data_p, uint16_t length) {
  uint8_t x;
  uint16_t crc = 0xFFFF;

  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
  }
  return crc;
}


bool Config::Save() {
#ifdef ESP8266
  EEPROM.begin(sizeof(Config));
#endif

  // Init pointer
  uint8_t * p = (uint8_t *) this ;

  // Init CRC
  this->crc = crc16(p, (uint8_t*)&this->crc - (uint8_t*)this);

  //For each byte of Config object
  for (uint16_t i = 0; i < sizeof(Config); ++i) EEPROM.write(i, *(p + i));

#ifdef ESP8266
  EEPROM.end();
#endif

  return Load();
}


bool Config::Load() {
#ifdef ESP8266
  EEPROM.begin(sizeof(Config));
#endif

  //tmpConfig will be used to load EEPROM datas
  Config tmpConfig;

  //create pointer tmpConfig
  uint8_t * p = (uint8_t *) &tmpConfig ;

  // For size of Config, read bytes
  for (uint16_t i = 0; i < sizeof(Config); ++i) *(p + i) = EEPROM.read(i);

#ifdef ESP8266
  EEPROM.end();
#endif


  // Check CRC
  if (crc16(p, (uint8_t*)&tmpConfig.crc - (uint8_t*)&tmpConfig) == tmpConfig.crc) {
    *this = tmpConfig;
    return true;
  }

  return false;
}

//------------------------------------------
//simple function that convert an hexadecimal char to byte
byte Config::AsciiToHex(char c) {
  return (c < 0x3A) ? (c - 0x30) : (c > 0x60 ? c - 0x57 : c - 0x37);
}

//------------------------------------------
// Function to decode https FingerPrint String into array of 20 bytes
bool Config::FingerPrintS2A(byte* fingerPrintArray, const char* fingerPrintToDecode) {

  if (strlen(fingerPrintToDecode) < 40) return false;

  byte arrayPos = 0;
  for (byte i = 0; i < strlen(fingerPrintToDecode); i++) {

    if (fingerPrintToDecode[i] != ' ' && fingerPrintToDecode[i] != ':' && fingerPrintToDecode[i] != '-') {
      fingerPrintArray[arrayPos / 2] += AsciiToHex(fingerPrintToDecode[i]);
      if (arrayPos % 2 == 0) fingerPrintArray[arrayPos / 2] *= 0x10;
      arrayPos++;
    }
    if (arrayPos == 40) return false;
  }

  return true;
}
//------------------------------------------
// Function that convert fingerprint Array to char array (with separator) (char array need to be provided)
char* Config::FingerPrintA2S(char* fpBuffer, byte* fingerPrintArray, char separator) {

  fpBuffer[0] = 0;

  for (byte i = 0; i < 20; i++) {
    sprintf_P(fpBuffer, PSTR("%s%02x"), fpBuffer, fingerPrintArray[i]);
    if (i != 19 && separator != 0) {
      fpBuffer[strlen(fpBuffer) + 1] = 0;
      fpBuffer[strlen(fpBuffer)] = separator;
    }
  }
  return fpBuffer;
}

void Config::InitWebServer(AsyncWebServer &server, bool &shouldReboot) {

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), (const uint8_t*)confightmlgz, sizeof(confightmlgz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/gc", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetJSON());
  });

  server.on("/sc", HTTP_POST, [this, &shouldReboot](AsyncWebServerRequest * request) {
    shouldReboot = SetFromParameters(request);
  });

  server.on("/wnl", HTTP_GET, [this](AsyncWebServerRequest * request) {

    int8_t n = WiFi.scanComplete();
    if (n == -2) {
      request->send(200, F("text/json"), F("{\"r\":-2,\"wnl\":[]}"));
      WiFi.scanNetworks(true);
    }
    else if (n == -1) {
      request->send(200, F("text/json"), F("{\"r\":-1,\"wnl\":[]}"));
    }
    else {
      String networksJSON(F("{\"r\":"));
      networksJSON = networksJSON + n + F(",\"wnl\":[");
      for (byte i = 0; i < n; i++) {
        networksJSON = networksJSON + '"' + WiFi.SSID(i) + '"';
        if (i != (n - 1)) networksJSON += ',';
      }
      networksJSON += F("]}");
      request->send(200, F("text/json"), networksJSON);
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) WiFi.scanNetworks(true);
    }
  });
}

String Config::GetJSON() {

  //{"s":"Wifi","p":"password","h":"TotoPC",...}

  String gc = F("{\"s\":\"");
  //there is a predefined special password (mean to keep already saved one)
  gc = gc + ssid + F("\",\"p\":\"") + (__FlashStringHelper*)predefPassword + F("\",\"h\":\"") + hostname + '"';
  if (ip) gc = gc + F(",\"ip\":\"") + IPAddress(ip).toString() + '"';
  gc = gc + F(",\"gw\":\"") + IPAddress(gw).toString() + '"';
  gc = gc + F(",\"mask\":\"") + IPAddress(mask).toString() + '"';
  if (dns1) gc = gc + F(",\"dns1\":\"") + IPAddress(dns1).toString() + '"';
  if (dns2) gc = gc + F(",\"dns2\":\"") + IPAddress(dns2).toString() + '"';

  for (byte i = 0; i < MODEL_WFP; i++) {
    gc = gc + F(",\"fp") + (i + 1) + F("n\":\"") + fpNames[i] + '"';
  }

  gc += '}';

  return gc;
}

bool Config::SetFromParameters(AsyncWebServerRequest* request) {

  //temp config
  Config tempConfig;

  //Parse Parameters
  if (!request->hasParam(F("s"), true)) {
    request->send(400, F("text/html"), F("SSID missing"));
    return false;
  }
  else strcpy(tempConfig.ssid, request->getParam(F("s"), true)->value().c_str());

  if (request->hasParam(F("p"), true) && request->getParam(F("p"), true)->value().length() < sizeof(tempConfig.password)) strcpy(tempConfig.password, request->getParam(F("p"), true)->value().c_str());
  if (request->hasParam(F("h"), true) && request->getParam(F("h"), true)->value().length() < sizeof(tempConfig.hostname)) strcpy(tempConfig.hostname, request->getParam(F("h"), true)->value().c_str());

  //check for previous password ssid and apiKey (there is a predefined special password that mean to keep already saved one)
  if (!strcmp_P(tempConfig.password, predefPassword)) strcpy(tempConfig.password, password);

  IPAddress ipParser;
  if (request->hasParam(F("ip"), true) && ipParser.fromString(request->getParam(F("ip"), true)->value())) tempConfig.ip = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("gw"), true) && ipParser.fromString(request->getParam(F("gw"), true)->value())) tempConfig.gw = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("mask"), true) && ipParser.fromString(request->getParam(F("mask"), true)->value())) tempConfig.mask = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("dns1"), true) && ipParser.fromString(request->getParam(F("dns1"), true)->value())) tempConfig.dns1 = static_cast<uint32_t>(ipParser);
  if (request->hasParam(F("dns2"), true) && ipParser.fromString(request->getParam(F("dns2"), true)->value())) tempConfig.dns2 = static_cast<uint32_t>(ipParser);

  if (request->hasParam(F("fp1n"), true) && request->getParam(F("fp1n"), true)->value().length() < sizeof(fpNames[0])) strcpy(tempConfig.fpNames[0], request->getParam(F("fp1n"), true)->value().c_str());
  else tempConfig.fpNames[0][0] = 0;
  if (request->hasParam(F("fp2n"), true) && request->getParam(F("fp2n"), true)->value().length() < sizeof(fpNames[1])) strcpy(tempConfig.fpNames[1], request->getParam(F("fp2n"), true)->value().c_str());
  else tempConfig.fpNames[1][0] = 0;
  if (request->hasParam(F("fp3n"), true) && request->getParam(F("fp3n"), true)->value().length() < sizeof(fpNames[2])) strcpy(tempConfig.fpNames[2], request->getParam(F("fp3n"), true)->value().c_str());
  else tempConfig.fpNames[2][0] = 0;
  if (request->hasParam(F("fp4n"), true) && request->getParam(F("fp4n"), true)->value().length() < sizeof(fpNames[3])) strcpy(tempConfig.fpNames[3], request->getParam(F("fp4n"), true)->value().c_str());
  else tempConfig.fpNames[3][0] = 0;
  if (request->hasParam(F("fp5n"), true) && request->getParam(F("fp5n"), true)->value().length() < sizeof(fpNames[4])) strcpy(tempConfig.fpNames[4], request->getParam(F("fp5n"), true)->value().c_str());
  else tempConfig.fpNames[4][0] = 0;
  if (request->hasParam(F("fp6n"), true) && request->getParam(F("fp6n"), true)->value().length() < sizeof(fpNames[5])) strcpy(tempConfig.fpNames[5], request->getParam(F("fp6n"), true)->value().c_str());
  else tempConfig.fpNames[5][0] = 0;
  if (request->hasParam(F("fp7n"), true) && request->getParam(F("fp7n"), true)->value().length() < sizeof(fpNames[6])) strcpy(tempConfig.fpNames[6], request->getParam(F("fp7n"), true)->value().c_str());
  else tempConfig.fpNames[6][0] = 0;
  if (request->hasParam(F("fp8n"), true) && request->getParam(F("fp8n"), true)->value().length() < sizeof(fpNames[7])) strcpy(tempConfig.fpNames[7], request->getParam(F("fp8n"), true)->value().c_str());
  else tempConfig.fpNames[7][0] = 0;

  //then save
  bool result = tempConfig.Save();

  //Send client answer
  if (result) request->send(200);
  else request->send(500, F("text/html"), F("Configuration hasn't been saved"));

  return result;
}
