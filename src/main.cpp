#include "Plant.h"            //Include der eigenen Klassen
#include "GlobalHeader.h"     //Include des Globalen Headers für Globale Variablen/Definitionen

#include <Arduino.h>          //Arduino Standartfunktionen (z.B. digitalWrite(Pin,Status);)
#include <ArduinoJson.h>      //Für die Auswertung von Bluetoothdaten welche als JSON empfangen werden
#include <RTClib.h>           //Für den Betrieb der RTC Einheit und die Zeitformate            
#include <SPI.h>              //Für die Kommunikation mit dem RTC-Modul
#include <BluetoothSerial.h>  //Für die Bluetooth Kommunikation
#include <string.h>           //Für String-Operationen
#include <Preferences.h>      //Für Speicher-/Auslesevorgänge des Flash-EEPROM-Speichers

//--------------------------------------------------------------------
// MAIN VARIABLEN LISTE:

//Variable für RTC-Operationen
RTC_DS1307 rtc;

//Variable mit aktuellem Zeitstempel
DateTime CurrentTime;

//Counter für die Leuchtdauer der BT-LED
uint8_t Led_BT_count = 0;

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
  Tasksetup();

  //Real Time Clock wird initialisiert
  RTCSetup(&rtc);

  //Timer initialisieren
  Timersetup();

  //Ports Definieren
  Pump::Setup(PUMPE_OUT,LED_PUMP,VALVE1,VALVE2,VALVE3);

  //EEPROM 
  PLANT_EEPROM::Setup_and_Load(&Pflanzenliste);
}


 
void loop(){

  //Durchführung der Schleife = 1x pro Millisekunde
  if(milli_flag==true)
  {
    milli_flag = false;

    Pump::Activation(CurrentTime,&Pflanzenliste);
    Pflanzenliste.MilliSec_Loop();
  }


  //Durchführung der Schleife = 1x pro Sekunde
  if(Second_flag)
  {
    Second_flag = false;
    
    CurrentTime = rtc.now();

    if(NEW_Bluetoothdata)
    {
      JSON_CONVERTER::Convert_JSON_TO_DATA(&Pflanzenliste, &rtc);
      NEW_Bluetoothdata = false;
      Led_BT_count = BT_LED_SECONDS;
    }

    BluetoothLED(&Led_BT_count);

    //Serial Monitor Kontrolle
    Pflanzenliste.Serialprint_all_Elements();
    Pump::ControlTask();    
    
    //Full Timestamp
    Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+CurrentTime.timestamp(DateTime::TIMESTAMP_FULL)+"\n");
  }
  
}