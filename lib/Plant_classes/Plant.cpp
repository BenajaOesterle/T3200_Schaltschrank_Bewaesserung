#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include "Plant.h"

    

    void Plant::FillinData(uint32_t watering_time_in_msec, String Plantname, uint8_t Output_Port, uint8_t W_Hour, uint8_t W_Minute)
    {
        this->watering_time_in_msec = watering_time_in_msec;
        this->Plantname = Plantname;
        this->Output_Port = Output_Port;
        this->W_Hour = W_Hour;
        this->W_Minute = W_Minute;
        //Plant_counter++;
    }

    void Plant::Serial_print_plant()
    {
        Serial.print("  Name = ");
        Serial.println(Plantname);
        Serial.print("  OutputPort = ");
        Serial.println(Output_Port);
        Serial.print("  Hour = ");
        Serial.println(W_Hour);
        Serial.print("  Minute = ");
        Serial.println(W_Minute);
        Serial.print("  Wateringtime in MilliSeconds = ");
        Serial.println(watering_time_in_msec);
        Serial.print("  Needs to be watered = ");
        Serial.println(NeedsToBeWatered);
        Serial.println();
        
    }

//Plantlist--------------------------------------------------------------------------------------------
    
    uint8_t Plantlist::WateringCounter = 1;
    
    bool Plantlist::Create_new_Plant(uint32_t watering_time_in_msec, String Plantname, uint8_t Output_Port, uint8_t W_Hour, uint8_t W_Minute){
        Plant_counter++;

        //Waterspeed = variable
        //Plantlist_ARRAY[Plant_counter].FillinData(waterspeed,watering_time_in_sec,Plantname,Output_Port);

        //Waterspeed = const
        Plantlist_ARRAY[Plant_counter].FillinData(watering_time_in_msec,Plantname,Output_Port,W_Hour,W_Minute);
        return 1;
    }

    bool Plantlist::Create_new_Plant_with_NR(uint8_t NR, uint32_t watering_time_in_msec, String Plantname, uint8_t Output_Port, uint8_t W_Hour, uint8_t W_Minute){
        if(Plant_counter<NR)
        {
            Plant_counter = NR;
        }
        //Waterspeed = variable
        //Plantlist_ARRAY[Plant_counter].FillinData(waterspeed,watering_time_in_sec,Plantname,Output_Port);
        Serial.printf("Plantname C_N_P = %s\n", Plantname);
        //Waterspeed = const
        Plantlist_ARRAY[NR].FillinData(watering_time_in_msec,Plantname,Output_Port,W_Hour,W_Minute);
        return 1;
    }

    void Plantlist::Serialprint_all_Elements(){
        for(int i = 1; i<=Plant_counter; i++)
        {
            Plantlist_ARRAY[i].Serial_print_plant();
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
            Plantlist_ARRAY[i].NeedsToBeWatered = true;
        }
    }

    void Plantlist::MilliSec_Loop(){
    
        if(Pump::M_Control_active == false)
        {
            if(WateringCounter == 0){Serial.println("ERROR WateringCounter = 0");}
            else if(WateringCounter<=Plant_counter && Pump::Pump_availaible==true)
            {
                if(Plantlist_ARRAY[WateringCounter].NeedsToBeWatered == true)
                {
                    Pump::Start_Water_Process(
                                                WATERSPEED_PLANTS,
                                                Plantlist_ARRAY[WateringCounter].watering_time_in_msec, 
                                                Plantlist_ARRAY[WateringCounter].Output_Port
                                            );
                    Plantlist_ARRAY[WateringCounter].NeedsToBeWatered = false;
                }
                WateringCounter++;

            }
            else if(WateringCounter>Plant_counter&& Pump::Pump_availaible==true)
            {
                WateringCounter = 1;
            }
        }
    }
    