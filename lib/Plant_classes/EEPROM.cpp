#include <Arduino.h>
#include "Plant.h"
#include <Preferences.h>

void PLANT_EEPROM::Setup_and_Load(Plantlist *Pflanzenliste){
    
    //Objekt der Preferences.h Bibiliothek welches zum Lesen/Speichern von Daten im EEPROM genutzt wird
    Preferences prefs;

    //Speicheradresse
    int Adresse_Int = 0;
    char Adresse_Char[8];

    //Convertiere die Int Adresse in ein für die Preferences.h passendes Format
    itoa(Adresse_Int,Adresse_Char,10);
    
    //Speichere/Lade bis zur Command "prefs.end();" alles unter folgendem Codewort im EEPROM
    prefs.begin("Plantlist",false);

    //Übernehme die Anzahl an Pflanzen in die Pflanzenliste
    uint8_t tmp_Plant_Counter = prefs.getUShort(Adresse_Char,0);

    Serial.printf("Geladene Pflanzenanzahl = %d\n", tmp_Plant_Counter);
    
    //Diese 2 Zeilen erhöhen den Adresszähler identisch zum Schreibvorgang.
    Adresse_Int +=1; 
    itoa(Adresse_Int,Adresse_Char,10);

    //Auslesen aller Pflanzen die sich im EEPROM befinden.
    for(int i = 1; i<=tmp_Plant_Counter; i++)
    {
        Plant tmp_Plant;
        
        
        String Name = prefs.getString(Adresse_Char,"NONAME");
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);
        
        uint8_t Valve = prefs.getUShort(Adresse_Char,0);
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        uint8_t Counter_Wateringtimes = prefs.getUShort(Adresse_Char,0);
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        for(int y=1; y<=Counter_Wateringtimes; y++)
        {
            uint32_t Wateringtime = prefs.getUInt(Adresse_Char,0);
            Adresse_Int +=1; 
            itoa(Adresse_Int,Adresse_Char,10);

            uint8_t Hour = prefs.getUShort(Adresse_Char,0);
            Adresse_Int +=1; 
            itoa(Adresse_Int,Adresse_Char,10);

            uint8_t Minute = prefs.getUShort(Adresse_Char,0);
            Adresse_Int +=1; 
            itoa(Adresse_Int,Adresse_Char,10);

            Serial.print("Bewässerungszeit = ");
            Serial.println(Wateringtime);
            Serial.print("Stunde = ");
            Serial.println(Hour);
            Serial.print("Minute = ");
            Serial.println(Minute);

            tmp_Plant.Bewaesserungszeitpunkte_ARRAY[y].watering_time_in_msec = Wateringtime;
            tmp_Plant.Bewaesserungszeitpunkte_ARRAY[y].W_Minute = Minute;
            tmp_Plant.Bewaesserungszeitpunkte_ARRAY[y].W_Hour = Hour;
        }
        tmp_Plant.Plantname = Name;
        tmp_Plant.Output_Port = Valve;
        tmp_Plant.Counter_Wateringtimes = Counter_Wateringtimes;
        

        Serial.print("Name = ");
        Serial.println(Name);
        Serial.print("Ventil = ");
        Serial.println(Valve);
        
        Serial.printf("Stringlänge = %d\n\n", Adresse_Int);

        //Erstellen einer neuen Pflanze anhand ausgelesender Daten
        Pflanzenliste->Add_Plant(tmp_Plant);
    }
    
    //Beende das Operieren unter dem Codewort "Plantlist"
    prefs.end();
}




void PLANT_EEPROM::Save(Plantlist Pflanzenliste){

    //Objekt der Preferences.h Bibiliothek welches zum Lesen/Speichern von Datem im EEPROM genutzt wird
    Preferences prefs;
    int Adresse_Int = 0;
    char Adresse_Char[8];

    //Convertiere die Int Adresse in ein für die Preferences.h passendes Format
    itoa(Adresse_Int,Adresse_Char,10);
    
    //Speichere/Lade bis zur Command "prefs.end();" alles unter folgendem Codewort im EEPROM
    prefs.begin("Plantlist",false);

    //Speichere die neue Länge der Pflanzenliste als short unsigned integer Variable,
    //wenn diese nicht der alten länge entspricht. (0 = Standart Rückgabewert)
    Serial.printf("PFLANZENLISTE ANZAHL GESPEICHERT = %d\n",Pflanzenliste.Plant_counter);
    if(!(prefs.getUShort(Adresse_Char,0)==Pflanzenliste.Plant_counter))
        prefs.putUShort(Adresse_Char,Pflanzenliste.Plant_counter);

    //Erhöhe die Speicheradresse. Dateigrößen werden automatisch berücksichtigt
    Adresse_Int +=1; 
    itoa(Adresse_Int,Adresse_Char,10);

    //Alle Pflanzen der Pflanzenliste werden abgepeichert.
    for(uint8_t i = 1; i<=Pflanzenliste.Plant_counter; i++)
    {
        //Name soll abgespeichert werden
        //Ist der Name bereits an der Stelle im EEPROM vorhanden weil z.B. nur eine andere Pflanze geändert wurde,
        //schreibe den Namen nicht erneut um die Anzahl an 
        if(!(prefs.getString(Adresse_Char,"NONAME") == Pflanzenliste.Plantlist_ARRAY[i].Plantname))
        {
            //Durch putString wird der Speicherbedarf im EEPROM festgelegt, sowie der Adressraum
            prefs.putString(Adresse_Char,Pflanzenliste.Plantlist_ARRAY[i].Plantname);
            Serial.println("Write EEPROM Name");
        }  
        //Erhöhe die Adresse für die nächste zu speichernede Variable.
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        //Ausgangsport (Ventil Nummer) soll abgespeichert werden. (Vorgehen identisch zu Name)
        if(!(prefs.getUShort(Adresse_Char,0) == Pflanzenliste.Plantlist_ARRAY[i].Output_Port))
        {
            prefs.putUShort(Adresse_Char,Pflanzenliste.Plantlist_ARRAY[i].Output_Port);
            Serial.println("Write EEPROM Valve");
        }            
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        if(!(prefs.getUShort(Adresse_Char,0) == Pflanzenliste.Plantlist_ARRAY[i].Counter_Wateringtimes))
        {
            prefs.putUShort(Adresse_Char,Pflanzenliste.Plantlist_ARRAY[i].Counter_Wateringtimes);
            Serial.println("Write EEPROM Hour");
        }          
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        for(int y=1; y<=Pflanzenliste.Plantlist_ARRAY[i].Counter_Wateringtimes;y++)
        {
            //Bewässerungszeit soll abgespeichert werden. (Vorgehen identisch zu Name)
            if(!(prefs.getUInt(Adresse_Char,0) == Pflanzenliste.Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].watering_time_in_msec))
            {
                prefs.putUInt(Adresse_Char,Pflanzenliste.Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].watering_time_in_msec);
                Serial.println("Write EEPROM Wateringtime");
            }            
            Adresse_Int +=1; 
            itoa(Adresse_Int,Adresse_Char,10);

            if(!(prefs.getUShort(Adresse_Char,0) == Pflanzenliste.Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].W_Hour))
            {
                prefs.putUShort(Adresse_Char,Pflanzenliste.Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].W_Hour);
                Serial.println("Write EEPROM Hour");
            }          
            Adresse_Int +=1; 
            itoa(Adresse_Int,Adresse_Char,10);

            if(!(prefs.getUShort(Adresse_Char,0) == Pflanzenliste.Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].W_Minute))
            {
                prefs.putUShort(Adresse_Char,Pflanzenliste.Plantlist_ARRAY[i].Bewaesserungszeitpunkte_ARRAY[y].W_Minute);
                Serial.println("Write EEPROM Minute");
            }           
            Adresse_Int +=1; 
            itoa(Adresse_Int,Adresse_Char,10);
        }


    }

    Serial.printf("Stringlänge = %d\n", Adresse_Int);
    prefs.end();
}