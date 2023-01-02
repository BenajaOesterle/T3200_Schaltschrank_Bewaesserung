#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include <BluetoothSerial.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>

#include "Plant.h"

StaticJsonDocument<2048> JSON_CONVERTER::JSON_Data;

void Manually_Control(JsonObject M_Control, DateTime CurrentTime){
    uint8_t Pump = M_Control["Pumpe"];
    uint8_t Ventil1 = M_Control["Ventil1"];
    uint8_t Ventil2 = M_Control["Ventil2"];
    uint8_t Ventil3 = M_Control["Ventil3"];

    if(Pump>0 || Ventil1>0 || Ventil2>0|| Ventil3>0)
    {
        //Ausgabe der Empfangenen Zustände im Seriellen Monitor
        Serial.printf("MANUAL-OUTPUTS------------\n Pumpe = %d\n Ventil1 = %d\n Ventil2 = %d\n Ventil3 = %d\n\n",Pump,Ventil1,Ventil2,Ventil3);
        
        Pump::Pump_availaible = false;
        Pump::M_Control_active = true;
        Pump::Wateringtime_ms = MANUALLY_WATERINGTIME_IN_MS;
        Pump::Start_Time = CurrentTime;
        Pump::Waterspeed = Pump;
        Pump::Valve1_Condition = Ventil1;
        Pump::Valve2_Condition = Ventil2;
        Pump::Valve3_Condition = Ventil3;
    }
    else
    {
        Pump::M_Control_active = false;
        Pump::Waterspeed = Pump;
        Pump::Wateringtime_ms = 0;
        Pump::Valve1_Condition = 0;
        Pump::Valve2_Condition = 0;
        Pump::Valve3_Condition = 0;
    }

    
}

DateTime BuildTime(JsonObject Recieved_Time, RTC_DS1307 rtc)
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;

    if(Recieved_Time["Y"]<2100, Recieved_Time["Y"]>2021)
        {Year   = Recieved_Time["Y"];}
    else
        {Year = rtc.now().year();}

    if(Recieved_Time["M"]<13, Recieved_Time["M"]>0)
        {Month   = Recieved_Time["M"];}
    else
        {Month = rtc.now().month();}

    if(Recieved_Time["D"]<32, Recieved_Time["D"]>0)
        {Day   = Recieved_Time["D"];}
    else
        {Day = rtc.now().month();}

    if(Recieved_Time["h"]<24, Recieved_Time["h"]>=0)
        {Hour   = Recieved_Time["h"];}
    else
        {Hour = rtc.now().hour();}
    
    if(Recieved_Time["m"]<60, Recieved_Time["m"]>=0)
        {Minute = Recieved_Time["m"];}
    else
        {Minute = rtc.now().minute();}
    if(Recieved_Time["s"]<60, Recieved_Time["s"]>=0)
        {Second   = Recieved_Time["s"];}
    else
        {Second = rtc.now().minute();}
 
        
   
    DateTime NEWDate = {Year,Month,Day,Hour,Minute,Second};

    return NEWDate;
}


void JSON_CONVERTER::Convert_JSON_TO_DATA(Plantlist * Pflanzenliste, RTC_DS1307 *rtc_fnc){
    
    Serial.printf("\nConvert to JSON ---------------\n\n");
 
    JsonObject Recieved_Time        = JSON_Data["t"];           //Empfangene Zeit für die RTC
    JsonObject M_Control            = JSON_Data["m"];           //Daten für die manuelle Steuerung der Anlage
    int Recieved_WaterALL           = JSON_Data["w"];           //Kann zu testzwecken gesendet werden Falls > 0 werden alle angelegten Pflanzen bewässert
    bool delete_Plantlist           = JSON_Data["delete"];      //Sollte dieser Wert auf true sein, handelt es sich um einen Test ohne die Daten zu übernehemen
    int Counter                     = 0;

    //Erstellen einer neuen temporären Pflanzenliste um alle nicht empfangenen Daten auf Default zu haben.
    Plantlist tmp_Plantlist;

    //PLANTLIST------------------------------------------------------------------
    if(delete_Plantlist==false)
    {
        //Füge zur neu erstellten Pflanzenliste alle Pflanzen mit einer Bewässerungszeit von min. 1ms hinzu
        for(JsonObject Recieved_Plant : JSON_Data["p"].as<JsonArray>()){
                
            const char* Name = Recieved_Plant["n"]; 
            uint32_t Wateringtime = Recieved_Plant["w"]; 
            int Ventil = Recieved_Plant["v"]; 
            int hour = Recieved_Plant["h"];
            int minute = Recieved_Plant["m"];  


            if(Wateringtime>0)
            {
                Counter++;
                
                tmp_Plantlist.Create_new_Plant_with_NR(
                    Counter,
                    Wateringtime,
                    String(Name),
                    Ventil,
                    hour,
                    minute
                );
            }
        }
    }

    //Pflanzenliste übergeben und im EEPROM speichern 
    if(Counter>0 || delete_Plantlist == true)
    {
        tmp_Plantlist.Plant_counter = Counter;
        *Pflanzenliste = tmp_Plantlist; 
        PLANT_EEPROM::Save(tmp_Plantlist); 
    }
    
    //TIMESET------------------------------------------------------------------
    if(Recieved_Time["Y"]>2021)
    {
        //Wenn ein Zeitstempel empfangen wurde, wird dieser in ein DateTime Format convertiert
        DateTime NEWDate = BuildTime(Recieved_Time,*rtc_fnc);
        
        //Bei einer Abweichung von mehr als 60 sec wird die neue Zeit in die RTC-Einheit übernommen
        if( rtc_fnc->now().secondstime()<NEWDate.secondstime()-60
        ||  rtc_fnc->now().secondstime()>NEWDate.secondstime()+60
            )
        {
            rtc_fnc->adjust(NEWDate); 
        } 
    }

    //MANUALLY------------------------------------------------------------------
    if(!M_Control.isNull())
    {
        Serial.println("MANUELLE STEUERUNG");
        Manually_Control(M_Control, rtc_fnc->now());
    }
    

    //WATERALL------------------------------------------------------------------
    if(Recieved_WaterALL > 0)
    {
        Pflanzenliste->Water_ALL();
    }
}