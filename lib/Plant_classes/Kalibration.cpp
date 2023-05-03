#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include "Plant.h"


uint16_t Kalibration_Mode::Savety_Counter = 0;
uint8_t Kalibration_Mode::Active = 0;
uint8_t Kalibration_Mode::Valve = 0;
Plantlist Kalibration_Mode::Kalibration_Pt_List;
Plantlist Kalibration_Mode::tmp_Plantlist;

void Kalibration_Mode::Kalibration_Loop(Plantlist *CurrPlantlist, DateTime CurrTime){
    //In Active ==1 wird die Kalibrierung mit dem jeweiligen Ventil initialisiert und 
    //die aktuelle gegen eine Klaibrierungspflanzenliste ausgetauscht.
    if(Active==1)
    {
        Active = 2;
        tmp_Plantlist = *CurrPlantlist;
        Savety_Counter = 0;

        Plant Pflanze;
        Pflanze.Plantname = "Kalb_Valve";
        Pflanze.Output_Port = Valve;
        Pflanze.New_Wateringtime(CurrTime.hour(),CurrTime.minute(),KALIBRATION_TIME);
        Kalibration_Pt_List.Add_Plant(Pflanze);

        *CurrPlantlist = Kalibration_Pt_List;
    }
    //In Active==2 wird nach beendigung der Pumpe und einem Sicherheitstimer 
    //die vorherige Konfiguration wieder in den Ablauf geladen
    if(Active==2)
    {
        if(Pump::Pump_availaible==true)
        {
            Savety_Counter++;
        }
        if(Savety_Counter>=1000)
        {
            Active = 0;
            *CurrPlantlist = tmp_Plantlist;
            Kalibration_Mode::Savety_Counter = 0;
            Kalibration_Pt_List.Plant_counter = 0;
        }
    }   
}