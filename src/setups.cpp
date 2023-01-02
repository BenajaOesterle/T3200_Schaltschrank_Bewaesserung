#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>

#include "GlobalHeader.h"


//internal Variables
unsigned long millis_tmp_val = 0;
uint16_t MilliSecCounter = 0;
bool intern_OneMilliSecflag = false;


void Outputsetup()
{
  pinMode(PUMPE_OUT, OUTPUT);
  pinMode(VALVE1, OUTPUT);
  pinMode(VALVE2, OUTPUT);
  pinMode(VALVE3, OUTPUT);
  pinMode(LED_PUMP, OUTPUT);
  pinMode(LED_BT_Recieve, OUTPUT);
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, HIGH); 
  digitalWrite(VALVE3, HIGH);
  digitalWrite(LED_PUMP, HIGH);   
  digitalWrite(LED_BT_Recieve, LOW);     
}



void RTCSetup(RTC_DS1307 *rtc_fnc)
{
  //RTC_DS1307 rtc_tmp = *rtc_fnc;
  #ifndef ESP8266
    while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  if (! rtc_fnc->begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (! rtc_fnc->isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc_fnc->adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if(rtc_fnc->now().year() < 2022){
    Serial.print("RTC Time is set\n Old Year= ");
    Serial.println(rtc_fnc->now().year());
    rtc_fnc->adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //*rtc_fnc = rtc_tmp;
}

