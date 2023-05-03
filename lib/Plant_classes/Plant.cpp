#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include "Plant.h"
#include <iostream>
#include <algorithm>

    
    void Wateringtimes::Serial_print_Wateringtimes(double Correction){
        uint64_t wt_time = watering_time_in_msec * (int)Correction;
        Serial.printf("   Correction = %lf   wateringtime in ms = %d   watering_ohne_Corr = %d",Correction,wt_time,watering_time_in_msec);
        Serial.printf("   Hour = %d   Minute = %d   Needs to be watered = %d Watered Today = %d\n",W_Hour,W_Minute,NeedsToBeWatered,Watered_Today);
    }

    void Sensor_Reading::Serial_print_Reading(){
        Serial.printf("  Readingvalue = %d ",Reading_Value);
        Serial.println("Reading Time = " + Readingtime.timestamp(DateTime::TIMESTAMP_FULL));
    }

    void Temperatures::Serial_print(){
        Serial.printf("  Temperatur = %f°C ",Temperatur);
        Serial.println("Reading Time = " + Reading_Time.timestamp(DateTime::TIMESTAMP_FULL));
    }

    /*
    void Plant::FillinData(uint32_t watering_time_in_msec, String Plantname, uint8_t Output_Port, uint8_t W_Hour, uint8_t W_Minute)
    {

        this->Plantname = Plantname;
        this->Output_Port = Output_Port;
        //Plant_counter++;
    }
    */

    void Plant::New_Wateringtime(uint8_t W_Hour, uint8_t W_Min, uint32_t wateringtime){
        if(Counter_Wateringtimes<ARRAYSIZE_WATERINGTIMES-1)
        {
            Counter_Wateringtimes++;
            Bewaesserungszeitpunkte_ARRAY[Counter_Wateringtimes].watering_time_in_msec = wateringtime;
            Bewaesserungszeitpunkte_ARRAY[Counter_Wateringtimes].W_Hour = W_Hour;
            Bewaesserungszeitpunkte_ARRAY[Counter_Wateringtimes].W_Minute = W_Min;
        }
    }

    void Plant::Serial_print_plant(float Temp_Correction, DateTime CurrentTime)
    {
        Serial.print("\n  Name = ");
        Serial.print(Plantname);
        Serial.print("  OutputPort = ");
        Serial.println(Output_Port); 

        Serial.println("  Wateringtimes------");
        for(int i=1; i<=Counter_Wateringtimes; i++)
        {
            Bewaesserungszeitpunkte_ARRAY[i].Serial_print_Wateringtimes(Set_Correction(Temp_Correction,CurrentTime));
        }
        
        Serial.println("Readings----------------");

        for(int i = 1; i<=Counter_Sensorreadings; i++)
        {
            Sensorreadings_ARRAY[i].Serial_print_Reading();
        }

        Serial.println("-----------------------------------------");
               
    }


    void Plant::Add_New_Reading(uint16_t Sensorreading, DateTime CurrentTime)
    {
        if(Kalibration_Mode::Active==0)
        {
            Serial.println("New Reading = " + Sensorreading);

            if(Counter_Sensorreadings<1 
            || (Sensorreadings_ARRAY[Counter_Sensorreadings].Readingtime.secondstime()+REDUNDAT_MESSAGE)<CurrentTime.secondstime())
            {
                //Pump LED Blinkt 2x kurz auf
                Pump::LED_Number_Blinks = 4;

                if(Counter_Sensorreadings < SENSORREADINGS_QUANTITY-1)
                {
                    Counter_Sensorreadings++;
                    Sensorreadings_ARRAY[Counter_Sensorreadings].Reading_Value = Sensorreading;
                    Sensorreadings_ARRAY[Counter_Sensorreadings].Readingtime = CurrentTime;
                }
                else if(Counter_Sensorreadings >= SENSORREADINGS_QUANTITY-1){
                    for(uint8_t i = 1; i<Counter_Sensorreadings; i++)
                    {
                        Sensorreadings_ARRAY[i] = Sensorreadings_ARRAY[i+1];
                    }
                    Sensorreadings_ARRAY[Counter_Sensorreadings].Reading_Value = Sensorreading;
                    Sensorreadings_ARRAY[Counter_Sensorreadings].Readingtime = CurrentTime;
                }               
            }
        }
    }




float Plant::Set_Correction(float Square_Temp, DateTime CurrentTime){
    if(Kalibration_Mode::Active==0)
    {
        //Ist Messwert noch aktuell
        int Difference = CurrentTime.secondstime() - Sensorreadings_ARRAY[Counter_Sensorreadings].Readingtime.secondstime();
        float Correction = 0.0;

        //Ist Messwert Plausibel
        if(Difference<MEASURING_VALUE_IS_OLD 
        && Sensorreadings_ARRAY[Counter_Sensorreadings].Reading_Value>MEASURING_POSSIBLE_LOWER_LIMIT
        && Sensorreadings_ARRAY[Counter_Sensorreadings].Reading_Value<MEASURING_POSSIBLE_UPPER_LIMIT)
        {
        Correction = (float(Sensorreadings_ARRAY[Counter_Sensorreadings].Reading_Value)/MEASURING_SQUAREVALUE);
        Serial.printf("Correction ohne Tmp = %f",Correction);
        }

        //Ist Temperatur Plausibel
        if(Square_Temp >= MEASURING_TEMP_UNPLAUSIBLE)
            Square_Temp = 1.0;
        else
            Square_Temp = Square_Temp/MEASURING_SQUARETEMP;

        if(Correction>0.0)
        {
            Correction = ((Correction * MEASURING_WEIGHTING) + (Square_Temp*(10.0 - MEASURING_WEIGHTING))) / 10;
            return Correction;
            Serial.printf("Correction mit Tmp = %f",Correction);
        }
        else 
            return 1.0;

    }
    else
        return 1.0;
}


//Plantlist--------------------------------------------------------------------------------------------

    void Plantlist::Add_Temperatur(float Temperature, DateTime CurrentTime){
    
        if(Kalibration_Mode::Active==0)
        {
            if(Temperatures_counter<1 
            || (Temperatur_List[Temperatures_counter].Reading_Time.secondstime()+REDUNDAT_MESSAGE)<CurrentTime.secondstime())
            {
                if(Temperatures_counter < SENSORREADINGS_QUANTITY-1)
                {
                    Temperatures_counter++;
                    Temperatur_List[Temperatures_counter].Temperatur = Temperature;
                    Temperatur_List[Temperatures_counter].Reading_Time = CurrentTime;
                }
                else if(Temperatures_counter >= SENSORREADINGS_QUANTITY-1){
                    for(uint8_t i = 1; i<Temperatures_counter; i++)
                    {
                        Temperatur_List[i] = Temperatur_List[i+1];
                    }
                    Temperatur_List[Temperatures_counter].Temperatur = Temperature;
                    Temperatur_List[Temperatures_counter].Reading_Time = CurrentTime;
                }    
            }
        }
    }

    float Plantlist::Square_Temperature(DateTime CurrentTime){
        double tmp_Temperature = 0.0;
        uint8_t Counter_used_Temperatures = 0;

        for(int i=1; i<=Temperatures_counter;i++){
            if(Temperatur_List[i].Reading_Time.secondstime() > (CurrentTime.secondstime() - SECONDS_READING_IS_OLD))
            {
                Counter_used_Temperatures++;
                if(Temperatur_List[i].Temperatur>1.0)
                {
                    tmp_Temperature += Temperatur_List[i].Temperatur;
                }
                else
                {
                    i = Temperatures_counter + 1;
                    tmp_Temperature = 0;
                }
            }
        }


        if(Counter_used_Temperatures>0)
        {
            tmp_Temperature = tmp_Temperature/Counter_used_Temperatures;
            //Serial.printf("Square_Temperature = %f°C\n", tmp_Temperature);
            return tmp_Temperature;
        }
        else
        {
            //Serial.printf("No Temperature");
            return 100;
        }
    }

    void Plantlist::Add_Plant(Plant Pflanze){

        if(Plant_counter<ARRAYSIZE_PLANTS-1)
        {
            Plant_counter++;
            Plantlist_ARRAY[Plant_counter] = Pflanze;
        }
    }

    void Plantlist::Serialprint_all_Elements(DateTime CurrentTime){
        if(Printintervall<PRINTINTERVALL)
        {
            Printintervall++;
        }
        else
        {
            Serial.println("Print Plantlist");
            Serial.printf("Plants = %dst\n", Plant_counter);
            for(int i = 1; i<=Plant_counter; i++)
            {
                Plantlist_ARRAY[i].Serial_print_plant(Square_Temperature(CurrentTime), CurrentTime);
            }
            /*
            Serial.println("Temperatures------------------");
            for(int i = 1; i<=Temperatures_counter; i++)
            {
                Temperatur_List[i].Serial_print(); 
            }
            */
            Serial.println("------------------------------");

            Printintervall = 0;
        }
    }

    void Plantlist::RemovePlant(String name_rmv){

        uint8_t Plantfound = SearchPlant(name_rmv);

        if(!(Plantfound==0))
        {
            for(int i = Plantfound; i < Plant_counter; i++)
            {
                Plantlist_ARRAY[i] = Plantlist_ARRAY[i+1];
                
            }
            
            Plant_counter--;


            Serial.println("Succesfully Removed");  
        }
        else
        {
            Serial.println("Plant remove failed! No valid name parameter");
        }
    }

    uint8_t Plantlist::SearchPlant(String name_rmv){

        uint8_t Plantfound = 0;

        if (!(name_rmv == "ERROR"))
        {
            for(int i = 1; i <= Plant_counter; i++)
            {
                if(Plantlist_ARRAY[i].Plantname == name_rmv)
                {
                    Plantfound = i;
                }
            }
            if(Plantfound > 0)
            {
                return(Plantfound);
            }
            if(Plantfound == 0)
            {
                Serial.println("Not availaible in the current list");
            }    
            
        }
        else
        {
            Serial.println("Plant search failed! No valid name parameter");
        }
        return(0);
    }


    void Plantlist::Water_ALL(){
        for(int i = 1; i<=Plant_counter; i++)
        {
            for(int y=1; y<=Plantlist_ARRAY[i].Counter_Wateringtimes; y++)
            {
                Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].NeedsToBeWatered = true;
            }
            
        }
    }

    void Plantlist::periodic_function(DateTime CurrentTime){

        if(CurrentTime!=Pump::CurrentTime)
        {
            Sec_Loop(CurrentTime);
            Pump::CurrentTime = CurrentTime;
        }

        
        if(Pump::Pump_availaible==false)
        {Pump::Ramp();}

        Pump::OutputControl(); 


        for(int x = 1; x<=Plant_counter; x++)
        {
            for(int y = 1; y<=Plantlist_ARRAY[x].Counter_Wateringtimes; y++)
            {
                if(CurrentTime.hour() == 0
                && CurrentTime.minute() == 0
                && CurrentTime.second() == 0)
                {
                    Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].Watered_Today = false;
                }
                else
                {
                    DateTime Plant_Water_Time = {CurrentTime.year(), CurrentTime.month(), CurrentTime.day(), Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].W_Hour, Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].W_Minute,1};
                    
                    if(Plant_Water_Time.secondstime()<=CurrentTime.secondstime()
                    && Plant_Water_Time.secondstime()>=(CurrentTime.secondstime()-60)
                    && Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].Watered_Today == false)
                    {
                        Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].NeedsToBeWatered = true;
                        Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].Watered_Today = true;
                    }
                }      
            }
            
        }
    }

    void Plantlist::Sec_Loop(DateTime CurrentTime){
        if(Pump::M_Control_active == false && Pump::Pump_availaible == true)
        {
            for(uint8_t x=1; x<=Plant_counter; x++)
            {
                for(uint8_t y=0; y<=Plantlist_ARRAY[x].Counter_Wateringtimes; y++)
                {
                    if(Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].NeedsToBeWatered == true && Pump::Pump_availaible == true)
                    {
                        Serial.print("Correction Pump Setup = ");
                        Serial.println(Plantlist_ARRAY[x].Set_Correction(Square_Temperature(CurrentTime),CurrentTime));
                        Serial.print("Wateringime Pump Setup = ");
                        Serial.println(Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].watering_time_in_msec);
                        
                        Pump::Start_Water_Process(
                            WATERSPEED_PLANTS,
                            (Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].watering_time_in_msec * Plantlist_ARRAY[x].Set_Correction(Square_Temperature(CurrentTime),CurrentTime)),
                            Plantlist_ARRAY[x].Output_Port
                        );
                        Plantlist_ARRAY[x].Bewaesserungszeitpunkte_ARRAY[y].NeedsToBeWatered = false;
                    }
                }
                //Serial.printf("Pflanzennummer = %d\n",x);
                //Serial.printf("Correction = %f\n", Plantlist_ARRAY[x].Set_Correction(Square_Temperature(CurrentTime),CurrentTime));
            }
        }
    }

    