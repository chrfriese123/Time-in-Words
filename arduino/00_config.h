#ifndef __CONFIG_H_
#define __CONFIG_H_

/***********************************************************************
Key libraries needed for any other file
***********************************************************************/

#include <Wire.h>
#include "SPI.h"
#include "I2Cbus.h"          // I2Cbus library

#include <time.h>            // time() ctime()
time_t nowWIFI;                  // this is the epoch
tm * tmWIFI;                     // the structure tm holds time information in a more convient way

uint16_t CYC_UPDATE = 2000;
uint32_t counterData;        //replace timer function

/***********************************************************************
Watchdog setup & task handler (only ESP32)
***********************************************************************/

TaskHandle_t NeopixelLoopTask;

#include <esp_task_wdt.h>
#include <esp_err.h>

#define WDT_TIMEOUT               32 //Seconds

void wdtSetup() {
  ESP_ERROR_CHECK(esp_task_wdt_init(WDT_TIMEOUT, true));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));                 //add current thread to WDT watch  
}

void wdtReset() {
  vTaskDelay(0);
  esp_task_wdt_reset();
}
  
//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

#define ESP_getChipId()           ((uint32_t)ESP.getEfuseMac())

/***********************************************************************
01.1 Filesystem
***********************************************************************/
#include "FS.h"
  
// Valid setup for esp32 core from release 2.09 onwards
#include <LittleFS.h>
FS* filesystem =                  &LittleFS;
#define FileFS                    LittleFS
#define FS_Name                   "LittleFS"

/***********************************************************************
01.2 JSON config file
***********************************************************************/

#include <ArduinoJson.h>

//StaticJsonDocument<1024>              systemDoc;
String configJSON = "";
DynamicJsonDocument                   configDoc(2000);
//extern void     validate_json();
extern bool parseJSON();
  
char dummyJSON[] = "{\"General\":{\
\"tagStunde\":8,\
\"tagMinute\":0,\
\"tagBunt\":1,\
\"tagHell\":50,\
\"tagFarbe\":[10,100,200],\
\"nachtStunde\":21,\
\"nachtMinute\":0,\
\"nachtBunt\":1,\
\"nachtHell\":10,\
\"nachtFarbe\":[200,100,10]\
}\
}";

/***********************************************************************
03 WIFI 
***********************************************************************/

#define _ESPASYNC_WIFIMGR_LOGLEVEL_    2
#define TIME_BETWEEN_CONFIG_PORTAL_LOOP            1L

#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

extern bool activeAP;

// SSID and PW for Config Portal
String  ssid = "TC_" + String(ESP_getChipId(), HEX);

#define NUM_WIFI_CREDENTIALS          2

// SSID and PW for your Access Points
//String Router_SSID[NUM_WIFI_CREDENTIALS] = {"CFVV_IoT","Saumur"};
//String Router_Pass[NUM_WIFI_CREDENTIALS] = {"Hchr2023friS&","Rf1012!2145"};
String Router_SSID[NUM_WIFI_CREDENTIALS] = {" "," "};
String Router_Pass[NUM_WIFI_CREDENTIALS] = {" ", " "};


// You only need to format the filesystem once
#define FORMAT_FILESYSTEM           false

#define MIN_AP_PASSWORD_SIZE        8

#define SSID_MAX_LEN                32
#define PASS_MAX_LEN                64

typedef struct {
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct {
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

#define TZNAME_MAX_LEN                50
#define TIMEZONE_MAX_LEN              50

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
  char TZ_Name[TZNAME_MAX_LEN];     // "Europe/Berlin"
  char TZ[TIMEZONE_MAX_LEN];        // "CET-1CEST,M3.5.0,M10.5.0/3"
  uint16_t checksum;
} WM_Config;

WM_Config                             WM_config;

#define  WIFI_FILENAME                F("/wifi_cred.dat")
#define  CONFIG_FILENAME              F("/configuration.dat")

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;
#define USE_AVAILABLE_PAGES           false
#define USE_STATIC_IP_CONFIG_IN_CP    false
#define USE_CLOUDFLARE_NTP            false
#define USING_CORS_FEATURE            false
#define USE_DHCP_IP                   true
#define USE_CONFIGURABLE_DNS          false

// Just use enough to save memory. On ESP8266, can cause blank ConfigPortal screen if using too much memory
#define USING_AFRICA                  false
#define USING_AMERICA                 false
#define USING_ANTARCTICA              false
#define USING_ASIA                    false
#define USING_ATLANTIC                false
#define USING_AUSTRALIA               false
#define USING_EUROPE                  true
#define USING_INDIAN                  false
#define USING_PACIFIC                 false
#define USING_ETC_GMT                 false

#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager
#include <ESPmDNS.h>

#define HTTP_PORT                     80

AsyncWebServer                        webServer(HTTP_PORT);
AsyncDNSServer                        dnsServer;

WiFi_AP_IPConfig                      WM_AP_IPconfig;

extern void     wifi_setup();
extern void     wifi_check();

/***********************************************************************
04 Neopixel 
***********************************************************************/

#include <NeoPixelBus.h>

#define LED_PIN     16
#define LED_COUNT   250  //richtig: 228 aber etwas Puffer schadet nicht

extern uint16_t     tagZeit;
extern uint8_t      tagBunt;
extern uint16_t     tagHell;
extern uint8_t      tagFarbe[3];

extern uint16_t     nachtZeit;
extern uint8_t      nachtBunt;
extern uint16_t     nachtHell;
extern uint8_t      nachtFarbe[3];

extern void         neopixel_setup();
extern void         neopixel_loop( void * pvParameters );
extern void         neopixel_set();
extern void         neopixel_test();

/***********************************************************************
05 RTC
***********************************************************************/

#include "RTClib.h"

extern uint8_t   aktStunde;
extern uint8_t   aktMinute;

extern void      rtc_setup();
extern void      rtc_read();

#endif
