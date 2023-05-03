#include "Plant.h"            //Include der eigenen Klassen
#include "GlobalHeader.h"     //Include des Globalen Headers für Globale Variablen/Definitionen

#include <Arduino.h>          //Arduino Standartfunktionen (z.B. digitalWrite(Pin,Status);)
#include <ArduinoJson.h>      //Für die Auswertung von Bluetoothdaten welche als JSON empfangen werden
#include <RTClib.h>           //Für den Betrieb der RTC Einheit und die Zeitformate            
#include <SPI.h>              //Für die Kommunikation mit dem RTC-Modul
#include <BluetoothSerial.h>  //Für die Bluetooth Kommunikation
#include <string.h>           //Für String-Operationen
#include <Preferences.h>      //Für Speicher-/Auslesevorgänge des Flash-EEPROM-Speichers
// Include Libraries
#include <WiFi.h>
#include <esp_now.h>

//--------------------------------------------------------------------
// MAIN VARIABLEN LISTE:

//Variable für RTC-Operationen
RTC_DS1307 rtc;

//Variable mit aktuellem Zeitstempel
DateTime CurrentTime;

//Counter für die Leuchtdauer der BT-LED
uint8_t Led_BT_count = 0;

uint8_t On_Count = 0; 

//Enthält alle Daten der Pflanzen
Plantlist Pflanzenliste;

//---------------------------------------------------------------------

void setup() {
  
  //Serial start
  Serial.begin(115200);
  Serial.println("ESP Gestartet");

  //PinsInitianlisation
  Outputsetup();

  //Task mit Bluetoothfunktion wird initialisiert 
  Tasksetup1();

  //Real Time Clock wird initialisiert
  RTCSetup(&rtc);

  //Timer initialisieren
  Timersetup();

  //Ports Definieren
  Pump::Setup(PUMPE_OUT,LED_PUMP,VALVE1,VALVE2,VALVE3);

  //EEPROM 
  PLANT_EEPROM::Setup_and_Load(&Pflanzenliste);

  JSON_CONVERTER::MAC_from_this_Device = Initial_ESP_NOW();
}


 

void loop(){

  //Durchführung der Schleife = 1x pro Millisekunde
  if(milli_flag==true)
  {
    milli_flag = false;

    Pflanzenliste.periodic_function(CurrentTime);
    //Pflanzenliste.MilliSec_Loop();
    Kalibration_Mode::Kalibration_Loop(&Pflanzenliste,CurrentTime);  
  }


  //Durchführung der Schleife = 1x pro Sekunde
  if(Second_flag)
  {
    Second_flag = false;
    
    CurrentTime = rtc.now();

    if(JSON_CONVERTER::NEW_Bluetoothdata)
    {
      Serial.println("New BT-Data--------------------------");
      JSON_CONVERTER::Convert_JSON_TO_DATA_BT(&Pflanzenliste, &rtc, CurrentTime);
      JSON_CONVERTER::NEW_Bluetoothdata = false;
      Led_BT_count = BT_LED_SECONDS;
    }
    if(JSON_CONVERTER::NEW_ESPNOW_Data)
    {
      Serial.println("New ESPNOW-Data--------------------------");
      JSON_CONVERTER::Convert_JSON_TO_DATA_ESPNOW(&Pflanzenliste, CurrentTime);
      JSON_CONVERTER::NEW_ESPNOW_Data = false;
    }

    BluetoothLED(&Led_BT_count);

    if(On_Count<MASTER_CALL_BC_TIME)
    {
      On_Count++;
      JSON_CONVERTER::RangeTest_STATUS = true;
      //broadcast(JSON_CONVERTER::MAC_to_JSON_String());
    }
    //Serial Monitor Kontrolle
    Pflanzenliste.Serialprint_all_Elements(CurrentTime);
    Pump::ControlTask();
    if(Kalibration_Mode::Active != 0)
    {
      Serial.printf("KalibrationMode Active = %d\n", Kalibration_Mode::Active);
    }    
    
    ESP_NOW_Broadcast_cycle();
  }
  
}