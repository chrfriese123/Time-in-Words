//-------------------------------------------------
//
//  Testuhr v2
//
//-------------------------------------------------

// Funktionen:  - Wifi Verbindung
//              - Webseite bereitstellen für Konfiguration (Farbe & Helligkeit / Tag & Nacht)
//              - NTP Netzwerkzeit abrufen
//              - Uhrzeit anzeigen
// V2:
// - Add RTC module
// - Shift Neopixel display to task loop (independent of Wifi AP)
// - Listen to MDNS entry
//
// RGBW         
// Rot      max 100..120
// Grün     max 100..120    
// Blau     max 100..120    
// Weiss        

#include "00_config.h"

void setup() {
  setCpuFrequencyMhz(80);

  //Setup watchdog system
  counterData = millis();
  //wdtSetup();

  // Setup serial output
  serial_setup();
  wdtReset();

  // Setup Neopixel output
  neopixel_setup();
  wdtReset();

  // Setup file system handling
  fs_setup();
  wdtReset();  

  // Setup RTC read function
  rtc_setup();
  wdtReset();

   //Offload neopixel display to task (run independent on AP)
  xTaskCreatePinnedToCore(neopixel_loop,"Neopixel_Loop",10000,NULL,0,&NeopixelLoopTask,1);
  wdtReset();

  // Setup Wifi connection, potentially with Access Point
  wifi_setup();
  wdtReset();

  wdtSetup();

  Serial.println(F("Setup complete!\t"));
  Serial.println(F("\n#####################################\n"));
  Serial.println(F("Start TextClock\n"));
}

void loop() {
  
  //if (millis() - counterData > CYC_UPDATE) { 
    //wifi_check();
    //espDelay(10000);
    //neopixel_set();
//    counterData = millis();   
//  }
  delay (200);
  
  wdtReset();
}
