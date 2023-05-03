#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include <BluetoothSerial.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>

#include "Plant.h"

StaticJsonDocument<2048> JSON_CONVERTER::JSON_Data_BT;
StaticJsonDocument<2048> JSON_CONVERTER::JSON_Data_ESPNOW;

bool JSON_CONVERTER::NEW_Bluetoothdata = false;
bool JSON_CONVERTER::NEW_ESPNOW_Data = false;
bool JSON_CONVERTER::RangeTest_STATUS = false;

String JSON_CONVERTER::MAC_from_this_Device;


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


void JSON_CONVERTER::Convert_JSON_TO_DATA_BT(Plantlist * Pflanzenliste, RTC_DS1307 *rtc_fnc, DateTime CurrentTime){
    
    Serial.printf("\nConvert to JSON from BT---------------\n\n");
 
    JsonObject Recieved_Time        = JSON_Data_BT["t"];           //Empfangene Zeit für die RTC
    JsonObject M_Control            = JSON_Data_BT["m"];           //Daten für die manuelle Steuerung der Anlage
    int Recieved_WaterALL           = JSON_Data_BT["w"];           //Kann zu testzwecken gesendet werden Falls > 0 werden alle angelegten Pflanzen bewässert
    bool delete_Plantlist           = JSON_Data_BT["delete"];      //Sollte dieser Wert auf true sein, handelt es sich um einen Test ohne die Daten zu übernehemen
    bool calibrate                  = JSON_Data_BT["calibrate"];
    int Calib_Valve                 = JSON_Data_BT["v"];
    int Counter                     = 0;


    if(!calibrate)
    {
        //Erstellen einer neuen temporären Pflanzenliste um alle nicht empfangenen Daten auf Default zu haben.
        Plantlist tmp_Plantlist;

        const char* BTName = JSON_Data_BT["p"]["n"];

        Serial.println("Name =" + String(BTName));

        //PLANTLIST------------------------------------------------------------------
        //if(delete_Plantlist==false)
        //{
            
            //Füge zur neu erstellten Pflanzenliste alle Pflanzen mit einer Bewässerungszeit von min. 1ms hinzu
            for(JsonObject Recieved_Plant : JSON_Data_BT["p"].as<JsonArray>()){
                
                Plant tmp_Plant;    
                
                const char* Plantname = Recieved_Plant["n"]; 
                tmp_Plant.Plantname = Plantname;
                tmp_Plant.Output_Port = Recieved_Plant["v"]; 

                //Serial.println("Name = " + tmp_Plant.Plantname + " Outputport = \n" + tmp_Plant.Output_Port);

                for(JsonObject Recieved_Watering_Times : Recieved_Plant["b"].as<JsonArray>()){
                    uint32_t Wateringtime = Recieved_Watering_Times["w"]; 
                    int hour = Recieved_Watering_Times["h"];
                    int minute = Recieved_Watering_Times["m"];  
                    //Serial.println("Wateringtime = " + Wateringtime);
                    tmp_Plant.New_Wateringtime(hour,minute,Wateringtime);
                }
                tmp_Plant.Serial_print_plant(100.0, CurrentTime);
                tmp_Plantlist.Add_Plant(tmp_Plant);
            }
        //}

        //Pflanzenliste übergeben und im EEPROM speichern 
        //Pump LED Blinkt 2x kurz auf
        Pump::LED_Number_Blinks = 6;
        *Pflanzenliste = tmp_Plantlist; 
        Serial.println();
        Serial.printf("Vor Speichern Pflanzenanzahl = %d", tmp_Plantlist.Plant_counter);
        PLANT_EEPROM::Save(tmp_Plantlist); 
        
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
    }
    
    //MANUALLY------------------------------------------------------------------
    if(!M_Control.isNull())
    {
        Serial.println("MANUELLE STEUERUNG");
        Manually_Control(M_Control, rtc_fnc->now());
    }

    //CALIBRATION---------------------------------------------------------------
    if(calibrate && Kalibration_Mode::Active == 0)
    {
        Kalibration_Mode::Active = 1;
        Kalibration_Mode::Valve = Calib_Valve;
    }
    
    //WATERALL------------------------------------------------------------------
    if(Recieved_WaterALL > 0)
    {
        Pflanzenliste->Water_ALL();
    }
}


void JSON_CONVERTER::Convert_JSON_TO_DATA_ESPNOW(Plantlist * Pflanzenliste, DateTime CurrentTime){

    Serial.printf("\nConvert to JSON from ESPNOW---------------\n\n");

    Plantlist tmp_Plantlist = *Pflanzenliste;
 
    String MacAdresse = JSON_Data_ESPNOW["K"];

    //Wenn Mac Adressen übereinstimmen übernehme die einzelnen Parameter in das jeweilige Pflanzenobjekt mit dem passenden Outputport
    if(MacAdresse == MAC_from_this_Device)
    {
        for(JsonObject SensorValues : JSON_Data_ESPNOW["S"].as<JsonArray>())
        {
            uint8_t Port = SensorValues["P"];
            uint16_t Reading = SensorValues["R"];

            Serial.printf("Port = %d  Reading = %d\n", Port, Reading);

            for(int i=1; i<=tmp_Plantlist.Plant_counter; i++)
            {
                Serial.printf("OutputportListe = %d  SendedPort = %d\n", tmp_Plantlist.Plantlist_ARRAY[i].Output_Port, Port);
                
                if(tmp_Plantlist.Plantlist_ARRAY[i].Output_Port == Port)
                {
                    
                    tmp_Plantlist.Plantlist_ARRAY[i].Add_New_Reading(Reading, CurrentTime);
                }
            }
        }
        //Füge die aktuelle Temperatur der Pflanzenliste hinzu
        float Temperature = JSON_Data_ESPNOW["T"];
        Serial.printf("Temperatur = %f°C\n",Temperature);
        tmp_Plantlist.Add_Temperatur(Temperature,CurrentTime);

        *Pflanzenliste = tmp_Plantlist;
    }
    else
    {
        Serial.println("Die MAC Stimmt nicht!!!");
    }
}

//Gibt das passende JSON-String zurück für das koppeln eines Sensormoduls
//sowie für den Test der Reichweite
String JSON_CONVERTER::Master_Call(){
    StaticJsonDocument <1000> doc;
    String JSON_String = "";

    doc["MASTER"] = MAC_from_this_Device;
    doc["RangeTest"] = MAC_from_this_Device;      

    serializeJson(doc,JSON_String);

    return(JSON_String);
}

//Gibt das passende JSON-String zurück für Reichweitentests des Sensormoduls
String JSON_CONVERTER::RangeTest(){
    StaticJsonDocument <1000> doc;
    String JSON_String = "";

    doc["RangeTest"] = MAC_from_this_Device;   

    serializeJson(doc,JSON_String);

    return(JSON_String);
}