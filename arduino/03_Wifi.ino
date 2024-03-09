#include "00_config.h"

//byte mac[6];
//char APID[11];
String tmpJSONout;
bool activeAP;

/***********************************************************************
 Connect to WiFi access points
***********************************************************************/

uint8_t connectMultiWiFi() {
  // For ESP32, this better be 0 to shorten the connect time.
  // For ESP32-S2/C3, must be > 500
  #if ( USING_ESP32_S2 || USING_ESP32_C3 )
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           500L
  #else
    // For ESP32 core v1.0.6, must be >= 500
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           800L
  #endif

  uint8_t status;

  // Load stored data, the addAP ready for MultiWiFi reconnection
  loadConfigData();
      
  // Add AP credentials 
  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) ) {
      wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
    }
  }
    
  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( (i < 20) && (status != WL_CONNECTED) ) {
    status = WiFi.status();

    if (status == WL_CONNECTED) break;
    else delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);
    wdtReset();
    i++;
  }

  if ( status == WL_CONNECTED ) {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  } 
  return status;
}

/***********************************************************************
 Check WiFi status
***********************************************************************/
void wifi_check() {
  static ulong checkwifi_timeout    = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L

  current_millis = millis();
  
  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0)) {
    if ( (WiFi.status() != WL_CONNECTED) ) {
      Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
      connectMultiWiFi();
    }
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }
}

/**********************************************************************/

int calcChecksum(uint8_t* address, uint16_t sizeToCalc) {
  uint16_t checkSum = 0;
  
  for (uint16_t index = 0; index < sizeToCalc; index++) {
    checkSum += * ( ( (byte*) address ) + index);
  }

  return checkSum;
}

/***********************************************************************
 Load Sensor & WiFi configuration 
***********************************************************************/
bool loadConfigData() {
  //LOGERROR(F("Load Textclock Cfg File "));
  File file = FileFS.open(CONFIG_FILENAME, "r");
  if (file) {
    DeserializationError error = deserializeJson(configDoc, file);
    if (error or configDoc.isNull()) {
      LOGERROR(F("Failed to read Textclock Cfg file, using default configuration"));
      configJSON = String(dummyJSON).c_str();
    } else {
      LOGERROR(F("Successful read Textclock Cfg file"));
      configJSON = "";
      serializeJson(configDoc, configJSON);
    }
    file.close();
  } else {
    LOGERROR(F("Failed to open Textclock Cfg file, use default"));
    configJSON = String(dummyJSON).c_str(); //provide dummy / template data as response
  }
  parseJSON();

  //LOGERROR(F("Load WiFi Cfg File "));
  file = FileFS.open(WIFI_FILENAME, "r");
  memset((void *) &WM_config,       0, sizeof(WM_config));

  if (file) {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));

    file.close();
    
    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) ) {
      LOGERROR(F("WM_config checksum wrong"));
      return false;
    }
    LOGERROR(F("Successful read WiFi Cfg file"));
    return true;
  } else {
    LOGERROR(F("Failed to read WiFi Cfg file"));
    return false;
  }
}

/***********************************************************************
 Save Sensor & WiFi configuration 
***********************************************************************/
void saveConfigData() {
  
  //LOGERROR(F("Save Textclock Cfg File"));
  File file = FileFS.open(CONFIG_FILENAME, "w");
  if (file) {
    if (serializeJson(configDoc, file) == 0) {
      LOGERROR(F("Failed to write to Textclock Cfg file"));
    } else {
      LOGERROR(F("Successful write to Textclock Cfg file"));
    }
    file.close();
  } else {
    LOGERROR(F("Failed to open write Textclock Cfg file"));
  }

  //LOGERROR(F("Save WiFi Cfg File "));
  file = FileFS.open(WIFI_FILENAME, "w");
  if (file) {
    WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );
    
    file.write((uint8_t*) &WM_config, sizeof(WM_config));
    file.close();
    LOGERROR(F("Successful write to WiFi Cfg file"));
  } else {
    LOGERROR(F("Failed to write to WiFi Cfg file"));
  }
}

/***********************************************************************
 Setup wifi, load configuration, try to conenct and if needed start AP
 Setup ESP32 internal webserver
***********************************************************************/
void wifi_setup() {
 uint8_t status;
 
  //Local intialization. Once its business is done, there is no need to keep it around
  #if ( USING_ESP32_S2 || USING_ESP32_C3 ) 
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, NULL, ssid.c_str());
  #else
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, ssid.c_str());
  #endif
  
  ESPAsync_wifiManager.setDebugOutput(false);
  ESPAsync_wifiManager.setMinimumSignalQuality(-1); 

  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  ESPAsync_wifiManager.setConfigPortalTimeout(180);

  //Load configuration data (Wifi / Sensors)
  bool WifiDataLoaded = loadConfigData();

  //if data is available --> check only timezone
  if (WifiDataLoaded) {
    //If no timezine is set / found, use default
    if (!(strlen(WM_config.TZ_Name) > 0)) memcpy(WM_config.TZ_Name, "Europe/Berlin", sizeof("Europe/Berlin") - 1);
    //If no timezine is set / found, use default
    if (!(strlen(WM_config.TZ) > 0)) memcpy(WM_config.TZ, "CET-1CEST,M3.5.0,M10.5.0/3", sizeof("CET-1CEST,M3.5.0,M10.5.0/3") - 1);

  //if data is not available --> try default/preset values
  } else if ((Router_SSID[0] > "") || (Router_Pass[0] > "")) {
    memcpy(WM_config.TZ_Name, "Europe/Berlin", sizeof("Europe/Berlin") - 1);
    memcpy(WM_config.TZ, "CET-1CEST,M3.5.0,M10.5.0/3", sizeof("CET-1CEST,M3.5.0,M10.5.0/3") - 1);
  
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
      if (strlen(Router_SSID[i].c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1) strcpy(WM_config.WiFi_Creds[i].wifi_ssid, Router_SSID[i].c_str());
      else memcpy(WM_config.WiFi_Creds[i].wifi_ssid, Router_SSID[i].c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);
  
      if (strlen(Router_Pass[i].c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1) strcpy(WM_config.WiFi_Creds[i].wifi_pw, Router_Pass[i].c_str());
      else memcpy(WM_config.WiFi_Creds[i].wifi_pw, Router_Pass[i].c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  
    }
  
  //if data is not available and no default/preset values --> directly into AP mode
  } else {
    initialConfig = true;
  }

  configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  //save data set in new config file
  saveConfigData();

  //try connect or access point mode
  while (WiFi.status() != WL_CONNECTED) {

    //try to connect to either one of the APs
    status = connectMultiWiFi();

    //if not successful then create configuration portal 
    if (!(status == WL_CONNECTED)) {
      byte mac[6];
      char macAddr[18];
      WiFi.macAddress(mac);
      sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      Serial.println(macAddr);
      
      initialConfig = false;
      Serial.println(F("We haven't got any access point credentials, so get them now"));
      Serial.println(F("Starting configuration portal @ 192.168.4.1"));
      activeAP = true;
      
      // Starts an configuration point
      if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str())) {
        Serial.println(F("Not connected to WiFi but continuing anyway."));
      } else {
        Serial.println(F("WiFi connected...yeey :)"));
      }
      activeAP = false;
      memset(&WM_config, 0, sizeof(WM_config));
    
      for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
        String tempSSID = ESPAsync_wifiManager.getSSID(i);
        String tempPW   = ESPAsync_wifiManager.getPW(i);
    
        if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1) strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
        else memcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);
  
        if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1) strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
        else memcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  
      }

      String tempTZ   = ESPAsync_wifiManager.getTimezoneName();
      Serial.print(F("Timezone Name "));
      Serial.println(tempTZ);
      if (strlen(tempTZ.c_str()) < sizeof(WM_config.TZ_Name) - 1) strcpy(WM_config.TZ_Name, tempTZ.c_str());
      else memcpy(WM_config.TZ_Name, tempTZ.c_str(), sizeof(WM_config.TZ_Name) - 1);
      //If no timezine is set / found, use default
      if (!(strlen(WM_config.TZ_Name) > 0)) memcpy(WM_config.TZ_Name, "Europe/Berlin", sizeof("Europe/Berlin") - 1);
            
      const char * TZ_Result = ESPAsync_wifiManager.getTZ(WM_config.TZ_Name);
      Serial.print(F("TZ "));
      Serial.println(TZ_Result);
      if (strlen(TZ_Result) < sizeof(WM_config.TZ) - 1) strcpy(WM_config.TZ, TZ_Result);
      else memcpy(WM_config.TZ, TZ_Result, sizeof(WM_config.TZ) - 1);
      //If no timezine is set / found, use default
      if (!(strlen(WM_config.TZ) > 0)) memcpy(WM_config.TZ, "CET-1CEST,M3.5.0,M10.5.0/3", sizeof("CET-1CEST,M3.5.0,M10.5.0/3") - 1);
         

      configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
      saveConfigData();
    }    
  }

  //++++ Define MDNS repsonses ++++//
  
  if(!MDNS.begin("textuhr")) {
     Serial.println(F("Error starting mDNS"));
     return;
  }
  
  //++++ Define webserver repsonses ++++//
  
  //System configuration update
  webServer.on("/config.json", HTTP_GET, [](AsyncWebServerRequest * request) {   
    tmpJSONout = "";
    serializeJson(configDoc, tmpJSONout);
    request->send(200, "application/json; charset=utf-8\r\nCache-Control: no-cache\r\nX-Content-Type-Options: nosniff", tmpJSONout); 
  });

  webServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage1;

    if (request->hasParam("tagZeit")) {
      inputMessage1 = request->getParam("tagZeit")->value();
      int index = inputMessage1.indexOf(':');
      configDoc["General"]["tagStunde"] = inputMessage1.substring(0, index).toInt();
      configDoc["General"]["tagMinute"] = inputMessage1.substring(index+1).toInt();
       
    } else if (request->hasParam("tagBunt")) {
      configDoc["General"]["tagBunt"] = request->getParam("tagBunt")->value().toInt();
           
    } else if (request->hasParam("tagHell")) {
      configDoc["General"]["tagHell"] = request->getParam("tagHell")->value().toInt();
      tagHell = request->getParam("tagHell")->value().toInt();
      
    } else if (request->hasParam("tagFarbe")) {
      configDoc["General"]["tagFarbe"][0] = request->getParam("r")->value().toInt();
      configDoc["General"]["tagFarbe"][1] = request->getParam("g")->value().toInt();
      configDoc["General"]["tagFarbe"][2] = request->getParam("b")->value().toInt();
    
    } else if (request->hasParam("nachtZeit")) {
      inputMessage1 = request->getParam("nachtZeit")->value();
      int index = inputMessage1.indexOf(':');
      configDoc["General"]["nachtStunde"] = inputMessage1.substring(0, index).toInt();
      configDoc["General"]["nachtMinute"] = inputMessage1.substring(index+1).toInt();

    } else if (request->hasParam("nachtBunt")) { 
      configDoc["General"]["nachtBunt"] = request->getParam("nachtBunt")->value().toInt();
     
    } else if (request->hasParam("nachtHell")) {
      configDoc["General"]["nachtHell"] = request->getParam("nachtHell")->value().toInt();
      nachtHell = request->getParam("nachtHell")->value().toInt();
          
    } else if (request->hasParam("nachtFarbe")) {
      configDoc["General"]["nachtFarbe"][0] = request->getParam("r")->value().toInt();
      configDoc["General"]["nachtFarbe"][1] = request->getParam("g")->value().toInt();
      configDoc["General"]["nachtFarbe"][2] = request->getParam("b")->value().toInt();
    }
    
    configJSON = "";
    serializeJson(configDoc, configJSON);
    //Serial.println(configJSON);
    parseJSON();
    saveConfigData();
      
    request->send(200, "text/plain", "OK");
  });
  
  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String inputMessage;
    //bool triggerRestart = false; 
    if (request->hasParam("newConfigJSON")) {
      configJSON = request->getParam("newConfigJSON")->value();
      parseJSON();
      saveConfigData();
    } 
    request->send(FileFS, "/index.html");
  });
  
  // Restart ESP
  webServer.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    String inputMessage;
    request->send(FileFS, "/index.html");
    ESP.restart();
  });

  // Provide favicon.ico
  webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(FileFS, "/favicon.ico");
  });

  webServer.begin();
  
  MDNS.addService("http", "tcp", 80);
  
  Serial.print(F("HTTP server started @ "));
  Serial.println(WiFi.localIP());
}
