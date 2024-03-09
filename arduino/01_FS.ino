#include "00_config.h"

/***********************************************************************
 Prepare filesystem
***********************************************************************/

void fs_setup() {
   // Format FileFS if not yet
#ifdef ESP32
  if (!FileFS.begin(true)) {
#else
  if (!FileFS.begin()) {
    FileFS.format();
#endif
    Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));
  
    if (!FileFS.begin()) {     
      // prevents debug info from the library to hide err message.
      espDelay(100);
      
      Serial.println(F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));

      while (true) {
        espDelay(1);
      }
    }
  } else {
    Serial.println(F("LittleFS successfully started!")); 
  }
}

/***********************************************************************
 Parse and validate configuration JSON 
***********************************************************************/
//bool validateJSON(String inpJSON) {
bool parseJSON() {

  JsonObject jsObj;
  
  Serial.println(F("Start parsing"));
  
  DeserializationError error = deserializeJson(configDoc, configJSON);

  if (!configDoc.containsKey("General")) configDoc["General"] = "";
  jsObj = configDoc["General"];
    
  if (!jsObj.containsKey("tagStunde")) jsObj["tagStunde"] = 8;
  if (!jsObj.containsKey("tagMinute")) jsObj["tagMinute"] = 0;
  tagZeit = jsObj["tagStunde"].as<unsigned int>() * 60 + jsObj["tagMinute"].as<unsigned int>();

  if (!jsObj.containsKey("tagBunt")) jsObj["tagBunt"] = 1;
  tagBunt = jsObj["tagBunt"].as<unsigned int>();

  if (!jsObj.containsKey("tagHell")) jsObj["tagHell"] = 128;
  if (jsObj["tagHell"].as<unsigned int>() > 128) jsObj["tagHell"] = 128;
  tagHell = jsObj["tagHell"].as<unsigned int>();

  if (!jsObj.containsKey("tagFarbe")) jsObj["tagFarbe"] = '[100,100,180]';
  tagFarbe[0] = jsObj["tagFarbe"][0].as<unsigned int>();
  tagFarbe[1] = jsObj["tagFarbe"][1].as<unsigned int>();
  tagFarbe[2] = jsObj["tagFarbe"][2].as<unsigned int>();

  if (!jsObj.containsKey("nachtStunde")) jsObj["nachtStunde"] = 21;
  if (!jsObj.containsKey("nachtMinute")) jsObj["nachtMinute"] = 0;
  nachtZeit = jsObj["nachtStunde"].as<unsigned int>() * 60 + jsObj["nachtMinute"].as<unsigned int>();

  // Wenn Nacht Zeit kleiner Tag Zeit, dann auswechslen
  if (nachtZeit < tagZeit) {
    uint16_t temp = jsObj["nachtStunde"]; 
    jsObj["nachtStunde"] = jsObj["tagStunde"];
    jsObj["tagStunde"] = temp;

    temp = jsObj["nachtMinute"]; 
    jsObj["nachtMinute"] = jsObj["tagMinute"];
    jsObj["tagMinute"] = temp;
    
    tagZeit = jsObj["tagStunde"].as<unsigned int>() * 60 + jsObj["tagMinute"].as<unsigned int>();
    nachtZeit = jsObj["nachtStunde"].as<unsigned int>() * 60 + jsObj["nachtMinute"].as<unsigned int>();
  }

  if (!jsObj.containsKey("nachtBunt")) jsObj["nachtBunt"] = 1;
  nachtBunt = jsObj["nachtBunt"].as<unsigned int>();

  if (!jsObj.containsKey("nachtHell")) jsObj["nachtHell"] = 50;
  if (jsObj["nachtHell"].as<unsigned int>() > 128) jsObj["nachtHell"] = 128;
  nachtHell = jsObj["nachtHell"].as<unsigned int>();

  if (!jsObj.containsKey("nachtFarbe")) jsObj["nachtFarbe"] = '[20,10,20]';
  nachtFarbe[0] = jsObj["nachtFarbe"][0].as<unsigned int>();
  nachtFarbe[1] = jsObj["nachtFarbe"][1].as<unsigned int>();
  nachtFarbe[2] = jsObj["nachtFarbe"][2].as<unsigned int>();

  configJSON = "";
  serializeJson(configDoc, configJSON);
 
  return true;
}
