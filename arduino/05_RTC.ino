#include "00_config.h"

/***********************************************************************
 Global parameters
***********************************************************************/

RTC_DS3231 rtc;

uint8_t   aktStunde;
uint8_t   aktMinute;
uint16_t  dailyUpdate;

/***********************************************************************
 Setup RTC
***********************************************************************/

void rtc_setup() {
  rtc.begin();
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } 

  DateTime nowRTC = rtc.now();
  dailyUpdate = nowRTC.day();
  
  rtc_read();
}

/***********************************************************************
 Read RTC
***********************************************************************/

void rtc_read() {
  //Read the current RTC time
  DateTime nowRTC = rtc.now();

  //Read the current ESP32 time
  time(&nowWIFI);                       
  tmWIFI = localtime(&nowWIFI);
  
  // Once per day set RTC to NTP
  if ((tmWIFI->tm_year < 23) && (tmWIFI->tm_mday != dailyUpdate)) {
    //Repeat until valid timestamp
    Serial.print("Set to ESP32 time ");
    Serial.print(tmWIFI->tm_hour);
    Serial.print(":");
    Serial.println(tmWIFI->tm_min);
    while(tmWIFI->tm_hour*60+tmWIFI->tm_min == 0) {
      delay(1000);
      time(&nowWIFI);                       
      tmWIFI = localtime(&nowWIFI);
    }

    //Set RTC to ESP32 time
    rtc.adjust(DateTime(tmWIFI->tm_year+1900, tmWIFI->tm_mon+1, tmWIFI->tm_mday, tmWIFI->tm_hour, tmWIFI->tm_min, tmWIFI->tm_sec));
    DateTime nowRTC = rtc.now();

    dailyUpdate = nowRTC.day();
  }

  aktStunde = nowRTC.hour();
  aktMinute = nowRTC.minute(); 
}
