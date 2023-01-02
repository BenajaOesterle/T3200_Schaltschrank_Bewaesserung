#ifndef Plant_h
#define Plant_h
#include <Arduino.h>
#include <RTClib.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define ARRAYSIZE_PLANTS 8                  //Arraygröße der Pflanzenliste
#define PRESCALER_RAMP 2                    //Geschwindigkeit, mit der die Pumpe hochfährt (kleiner = schneller)
#define WATERSPEED_PLANTS 255               //Pumpengeschwindigkeit fest definieren
#define MANUALLY_WATERINGTIME_IN_MS 100000  //Nach Zeit x deaktiviert sich die manuelle Steuerung


//PLANT-----------------------------------------------------------------------------------------------------------

class Plant {
    private:
        

    public: 
        uint8_t W_Hour;                         //Stunde zu welcher bewässert wird
        uint8_t W_Minute;                       //Minute zu welcher bewässert wird
        uint8_t Output_Port;                    //Ventil Port an welchem die Zuleitung der Pflanze angeschlossen ist
        uint32_t watering_time_in_msec;         //Zeit wie lange die Pumpe bewässert werden soll
        String Plantname;                       //Name der Pflanze

        bool Watered_Today = false;             //Wird um 0:00 Uhr zurückgesetzt auf false und nach Bearbeitung durch die Pumpe auf true gesetzt
        bool NeedsToBeWatered = false;          //Wenn Bewässerungszeit erreich auf true bis Bewässerungsauftrag durchgeführt wurde
        
        //Gebe die Daten der Pflanze im Seriellen Monitor aus.
        void Serial_print_plant();

        //Übernehme die Daten in das Objekt und falls Daten fehlen treffe Standardannahmen. 
        void FillinData(uint32_t watering_time_in_msec = 0, String Plantname = "NOT_DEF", uint8_t Output_Port = 0, uint8_t W_Hour = 0, uint8_t W_Minute = 0);
};

//PLANTLIST-------------------------------------------------------------------------------------------------------

class Plantlist {
    public:
        Plant Plantlist_ARRAY[ARRAYSIZE_PLANTS];    //Array welches alle Pflanzen beinhaltet
        uint8_t Plant_counter = 0;                  //Anzahl an verwendeten Pflanzen

        static uint8_t WateringCounter;             //Beinhaltet die Aktuelle Array Nummer des zu prüfenden Elements (verwendet in MilliSec_Loop)
        
        //erstelle eine neue Pflanze und ergänze diese zur Liste
        bool Create_new_Plant(uint32_t watering_time_in_ms = 0, String Plantname = "NOT_DEF", uint8_t Output_Port = 0, uint8_t W_Hour = 0, uint8_t W_Minute = 0);

        //Erstelle eine neue Pflanze an einer ganz bestimmten Stelle des Arrays und passe den Plant_counter gegebenenfalls an
        bool Create_new_Plant_with_NR(uint8_t NR = 0,uint32_t watering_time_in_ms = 0, String Plantname = "NOT_DEF", uint8_t Output_Port = 0, uint8_t W_Hour = 0, uint8_t W_Minute = 0);

        //Gebe alle Elemente in der Pflanzenliste im Seriellen Monitor aus
        void Serialprint_all_Elements();

        //Lösche eine Pflanze mittels Namen.
        void RemovePlant(String name_rmv = "ERROR");

        //Suche eine Pflanze in der Liste und gebe deren Array Nummer zurück
        uint8_t SearchPlant(String name_rmv = "ERROR");

        //Bewässere alle Pflanzen 
        void Water_ALL();

        //Führe die Überprüfung der Pflanzenliste jede Millisekunde aus 
        void MilliSec_Loop();
};

//PUMP------------------------------------------------------------------------------------------------------------
class Pump{
    public:
        static uint8_t Valve1_Port;         //Ausgangsport Ventil1
        static uint8_t Valve2_Port;         //Ausgangsport Ventil2
        static uint8_t Valve3_Port;         //Ausgangsport Ventil3
        static uint8_t Output_Port;         //Ausgangsport Pumpe
        static uint8_t LED_Pump;            //Ausgangsport LED_Pumpe

        static bool M_Control_active;       //Status = Manueller Modus aktiv
        static DateTime CurrentTime;        //Aktueller Zeitstempel kann übergeben werden um RTC Zugriffe zu minimieren
        static DateTime Prev_Sec;           //Speichert die vorherige Sekunde ab und dient zur Synchronisierung der zur RTC Zeit hinzugefügten Millisekunden
        static bool Pump_availaible;        //Pumpe wird benutzt = false
        static uint8_t PrescalerRamp;       //Dient als Countervariable für die realisierung eines Prescalers für die Anlauframpe der Pumpe
        static uint16_t Counter_MS;         //Enthält die aktuelle Millisekundenzeit

        static uint8_t Waterspeed;          //Geschwindigkeitsvorgabe für den aktuellen Pumpvorgang wird hier abgelegt
        static uint32_t Wateringtime_ms;    //Dauer des aktuellen Pumpauftrags
        static DateTime Start_Time;         //Startzeitpunkt des aktuellen Pumpauftrags
        
        static uint8_t Waterspeed_out;      //PWM-Wert für den Pumpenausgang
        static uint8_t Valve1_Condition;    //aktueller Zustand Ventil1 (mehr als bool möglich für evtl. Fehlerfall)
        static uint8_t Valve2_Condition;    //aktueller Zustand Ventil2
        static uint8_t Valve3_Condition;    //aktueller Zustand Ventil3

        //Initialfunktion der Pumpen
        static void Setup(uint8_t Output_Port = 0, uint8_t LED_Pump = 0, uint8_t Valve1_Port = 0, uint8_t Valve2_Port = 0, uint8_t Valve3_Port = 0);

        //Pumpvorgang Startfunktion
        static void Start_Water_Process(uint8_t Waterspeed, uint32_t Wateringtime_ms = 0, uint8_t Valve = 0);

        //PWM-Rampe der Pumpe
        static void Ramp();

        //Gibt die aktuellen Daten im Seriellen Monitor aus
        static void ControlTask();

        //Steuert die Hardware an 
        static void OutputControl();

        //Wird millisekündlich ausgeführt 
        static void Activation(DateTime CurrentTime_input, Plantlist *Pflanzenliste);
};


class PLANT_EEPROM{
    public:
        static void Setup_and_Load(Plantlist *Pflanzenliste);
        static void Save(Plantlist Pflanzenliste);
};


class JSON_CONVERTER{
    public:
        static StaticJsonDocument<2048> JSON_Data;
        static void Convert_JSON_TO_DATA(Plantlist * Pflanzenliste, RTC_DS1307 *rtc_fnc);
};
#endif