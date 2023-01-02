#ifndef GlobalHeader_h
#define Globalheader_h

#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include <EEPROM.h>
#include <BluetoothSerial.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>

//#include "Plant.h"

//Variables-----------------------------------------------------------------

//OUTPUTS
#define PUMPE_OUT  27
#define LED_BT_Recieve 2
#define LED_PUMP 14
#define VALVE1 23 
#define VALVE2 5 
#define VALVE3 13
#define VALVE4 19

#define BT_LED_SECONDS 5

//Variables-ENDE------------------------------------------------------------


//Setup.cpp-----------------------------------------------------------------
void Outputsetup();
void RTCSetup(RTC_DS1307 *rtc_fnc);
//Setup-ENDE----------------------------------------------------------------


//Bluetooth.cpp-------------------------------------------------------------
void Tasksetup();
void BluetoothLED(uint8_t * Led_BT_count);
extern bool NEW_Bluetoothdata;
//Bluetooth-ENDE------------------------------------------------------------


//Timer_fnc.cpp-------------------------------------------------------------
extern bool milli_flag;
extern bool Second_flag;

void IRAM_ATTR onTimer();
void Timersetup();
bool OneSecEdge();
bool OneMilliSecEdge();
bool OneSecCounter();
//Timer_fnc.cpp-ENDE--------------------------------------------------------

#endif