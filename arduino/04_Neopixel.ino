#include "00_config.h"
#include "definitions.h"

/***********************************************************************
 Global parameters
***********************************************************************/

NeoPixelBus<NeoGrbwFeature, NeoEsp32I2s0Sk6812Method> strip(LED_COUNT, LED_PIN);

uint16_t  tagZeit;
uint8_t   tagBunt;
uint16_t  tagHell;
uint8_t   tagFarbe[3];
uint16_t  nachtZeit;
uint8_t   nachtBunt;
uint16_t  nachtHell;
uint8_t   nachtFarbe[3];

/***********************************************************************
 Setup neopixel
***********************************************************************/
void neopixel_setup() {
  RgbwColor pixelFarbeW(0, 0, 30, 0);
  RgbwColor pixelFarbeB(40, 10, 80, 0);
  RgbwColor pixelFarbeR(50, 0, 0, 0);
  
  strip.Begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.ClearTo(0);

  //Familie
  for (uint8_t i = 0; i <7; i++) {
    if (welcomeScreen[0][i]>250) break;
    strip.SetPixelColor(2*welcomeScreen[0][i], pixelFarbeW);
    strip.SetPixelColor(2*welcomeScreen[0][i]+1, pixelFarbeW);
  }
  //Friese
  for (uint8_t i = 0; i <7; i++) {
    if (welcomeScreen[1][i]>250) break;
    strip.SetPixelColor(2*welcomeScreen[1][i], pixelFarbeB);
    strip.SetPixelColor(2*welcomeScreen[1][i]+1, pixelFarbeB);
  }
  //D
  strip.SetPixelColor(2*85, pixelFarbeR);
  strip.SetPixelColor(2*85+1, pixelFarbeR);
  //und
  for (uint8_t i = 0; i <7; i++) {
    if (welcomeScreen[3][i]>250) break;
    strip.SetPixelColor(2*welcomeScreen[3][i], pixelFarbeW);
    strip.SetPixelColor(2*welcomeScreen[3][i]+1, pixelFarbeW);
  }
  //R
  strip.SetPixelColor(2*106, pixelFarbeR);
  strip.SetPixelColor(2*106+1, pixelFarbeR);
  
  strip.Show();            // Turn OFF all pixels ASAP
}

/***********************************************************************
 Setup neopixel
***********************************************************************/
void neopixel_test() {
  RgbwColor pixelFarbe(0, 0, 255, 0);
  
  //Familie
  for (uint8_t i = 0; i < 18; i++) {
    pixelFarbe = RgbwColor(0, 0, 0, (i+1)*10);
    
    strip.SetPixelColor(2*testScreen[i], pixelFarbe);
    strip.SetPixelColor(2*testScreen[i]+1, pixelFarbe);
  }
  
  strip.Show();            // Turn OFF all pixels ASAP
}


/***********************************************************************
 Display information
***********************************************************************/
void neopixel_loop( void *pvParameters ) {
  for(;;) {         //endless loop as task on CPU1 
    if (millis() - counterData > CYC_UPDATE) {  
      counterData = millis();   
      neopixel_set();
    }
    
    wdtReset(); 
  }
}

/***********************************************************************
 Display information
***********************************************************************/
void neopixel_set() {
  float helligkeit;
  bool plusStunde = false;
  uint8_t tmpArray[114];
  
  RgbwColor pixelRot(255,0,0,0);
  RgbwColor pixelFarbe(0,0,0,128);
  RgbwColor pixelSchwarz(0,0,0,0);

  rtc_read();
//  Serial.print(aktStunde);
//  Serial.print(":");
//  Serial.println(aktMinute);
  
  //Leeres temporäres Array erstellen
  memset(tmpArray,0,sizeof(tmpArray));
  
  //Helligkeit an Tageszeit anpassen
  if ((aktStunde*60+aktMinute > tagZeit) && (aktStunde*60+aktMinute < nachtZeit)) {
    if (tagBunt == 1) {
      pixelFarbe = RgbwColor((uint8_t)(tagFarbe[0]/2), (uint8_t)(tagFarbe[1]/2), (uint8_t)(tagFarbe[2]/2), 0);
    } else {
      pixelFarbe = RgbwColor(0, 0, 0, (uint8_t)tagHell);
    }
  } else {
    if (nachtBunt == 1) {      
      pixelFarbe = RgbwColor((uint8_t)(nachtFarbe[0]/2), (uint8_t)(nachtFarbe[1]/2), (uint8_t)(nachtFarbe[2]/2), 0);
    } else {
      pixelFarbe = RgbwColor(0, 0, 0, (uint8_t)nachtHell);
    }
  }

  //Halbe Helligkeit for "Es ist" und "Uhr"
  // "Es ist" immer darstellen
  for (uint8_t i = 0; i < 5; i++) {
    tmpArray[ledKoordinate[0][i]] = 1;
  }
  
  // "Uhr" darstellen wenn Minute = 0
  if (aktMinute == 0) {
    for (uint8_t i = 0; i < 3; i++) {
      tmpArray[ledKoordinate[25][i]] = 1;
    }
  }

  // Minuten darstellen
  for (uint8_t j = 0; j < 7; j++) {
    if (minuteDefinition[aktMinute][j]>90) break;
    
    //Wenn Minutendarstellung "drei(viertel)", "vor" oder "halb" enthält, dann die nächste Stundenzahl darstellen
    if ((minuteDefinition[aktMinute][j] == 20) || (minuteDefinition[aktMinute][j] == 22) || (minuteDefinition[aktMinute][j] == 24)) plusStunde = true;

    for (uint8_t i = 0; i < 7; i++) {
      if (ledKoordinate[minuteDefinition[aktMinute][j]][i]>250) break;
      tmpArray[ledKoordinate[minuteDefinition[aktMinute][j]][i]] = 1;
    }
  }  

  // Stunden umrechnen: nur 1-12, ggf. +1
  if (aktStunde == 0) aktStunde = 12;
  if (plusStunde) aktStunde++;
  if (aktStunde > 12) aktStunde = aktStunde - 12;
        
  for (uint8_t i = 0; i < 7; i++) {
    if (ledKoordinate[aktStunde][i]>250) break;
    tmpArray[ledKoordinate[aktStunde][i]] = 1;
  }

  //Übertrag temporäres Array in Neopixel Strip
  for (uint8_t i = 0; i < 114; i++) {
    if (tmpArray[i] == 1) {
      strip.SetPixelColor(2*i, pixelFarbe);
      strip.SetPixelColor(2*i+1, pixelFarbe);
    } else {
      strip.SetPixelColor(2*i, pixelSchwarz);
      strip.SetPixelColor(2*i+1, pixelSchwarz);
    }
  }
  
  //If Accesspoint Mode active --> top left pixel in red
  if (activeAP) {
    strip.SetPixelColor(6,pixelRot);
    strip.SetPixelColor(7,pixelRot);
  }
  strip.Show();       
  
  wdtReset();
}
