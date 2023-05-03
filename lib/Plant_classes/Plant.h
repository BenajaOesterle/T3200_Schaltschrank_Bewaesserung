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
#define ARRAYSIZE_WATERINGTIMES 8           
#define SENSORREADINGS_QUANTITY 8
#define ARRAYSIZE_TEMPERATURREADINGS 8
#define SECONDS_READING_IS_OLD 28800        //8h entspricht z.B. 8*3600 = 28800
#define REDUNDAT_MESSAGE 3                  //If after X secs a message vor the same sensor arrived its redundant
#define KALIBRATION_TIME 30000              //Time in ms
#define VALVES_COUNT 3                      //Anzahl an Ventilen
#define PRINTINTERVALL 3                    //Jede Xte Sekunde wird alles ausgegeben 
#define BLINKSPEED_LED 200                  //Alle X ms Wechselt LED im Zustand im Blinkmodus
#define MEASURING_VALUE_IS_OLD 216000       //Messwert ist nach X Sekunden veraltet
#define MEASURING_POSSIBLE_LOWER_LIMIT 100  //Mindestwert damit Messwert Plausibel ist
#define MEASURING_POSSIBLE_UPPER_LIMIT 4000 //Maximalwert ansonsten ist Messwet nicht Plausibel
#define MEASURING_SQUAREVALUE 2500.0        //Mittlere Feuchtigkeit
#define MEASURING_TEMP_UNPLAUSIBLE 100.0    //Wenn Temperatur höher ist, ist die Messung nicht Plausibel
#define MEASURING_SQUARETEMP 20.0           //Durchschnittstemperatur für Faktor 1.0
#define MEASURING_WEIGHTING 9.5             //Gewichtung des Feuchtigkeitsmesserts gegenüber der Temperatur von 10.0

//WATERINGTIMES---------------------------------------------------------------------------------------------------
class Wateringtimes{
    public: 
        uint8_t W_Hour;                         //Stunde zu welcher bewässert wird
        uint8_t W_Minute;                       //Minute zu welcher bewässert wird
        uint32_t watering_time_in_msec;         //Zeit wie lange die Pumpe bewässert werden soll
        
        bool Watered_Today = false;             //Wird um 0:00 Uhr zurückgesetzt auf false und nach Bearbeitung durch die Pumpe auf true gesetzt
        bool NeedsToBeWatered = false;          //Wenn Bewässerungszeit erreich auf true bis Bewässerungsauftrag durchgeführt wurde

        void Serial_print_Wateringtimes(double Correction);
};

//SENSORREADING---------------------------------------------------------------------------------------------------
class Sensor_Reading{                           
    public:
        uint16_t Reading_Value = 0;
        DateTime Readingtime;

        void Serial_print_Reading();
};

//TEMPERATURES----------------------------------------------------------------------------------------------------
class Temperatures{
    public:
        float Temperatur = 0.0;
        DateTime Reading_Time;

        void Serial_print();
};


//PLANT-----------------------------------------------------------------------------------------------------------

class Plant {
    public:
        
        Wateringtimes Bewaesserungszeitpunkte_ARRAY[ARRAYSIZE_WATERINGTIMES];
        uint8_t Counter_Wateringtimes = 0;

        Sensor_Reading Sensorreadings_ARRAY[SENSORREADINGS_QUANTITY];
        uint8_t Counter_Sensorreadings = 0;

        uint8_t Output_Port;                    //Ventil Port an welchem die Zuleitung der Pflanze angeschlossen ist
        String Plantname;                       //Name der Pflanze

        //Gebe die Daten der Pflanze im Seriellen Monitor aus.
        void Serial_print_plant(float Square_Temp, DateTime CurrentTime);

        void New_Wateringtime(uint8_t W_Hour, uint8_t W_Min, uint32_t wateringtime);

        void Add_New_Reading(uint16_t Sensorreading, DateTime CurrentTime);

        float Set_Correction(float Square_Temp, DateTime CurrentTime);

        //Übernehme die Daten in das Objekt und falls Daten fehlen treffe Standardannahmen. 
        //void FillinData(uint32_t watering_time_in_msec = 0, String Plantname = "NOT_DEF", uint8_t Output_Port = 0, uint8_t W_Hour = 0, uint8_t W_Minute = 0);
};

//PLANTLIST-------------------------------------------------------------------------------------------------------

class Plantlist {
    public:
        Plant Plantlist_ARRAY[ARRAYSIZE_PLANTS];    //Array welches alle Pflanzen beinhaltet
        uint8_t Plant_counter = 0;                  //Anzahl an verwendeten Pflanzen

        Temperatures Temperatur_List[ARRAYSIZE_TEMPERATURREADINGS];
        uint8_t Temperatures_counter = 0;
        void Add_Temperatur(float Temperature, DateTime CurrentTime);
        float Square_Temperature(DateTime CurrentTime);        

        //Füge eine Pflanze der Liste hinzu
        void Add_Plant(Plant Pflanze);


        //Gebe alle Elemente in der Pflanzenliste im Seriellen Monitor aus
        uint8_t Printintervall = 0;
        void Serialprint_all_Elements(DateTime CurrentTime);

        //Lösche eine Pflanze mittels Namen.
        void RemovePlant(String name_rmv = "ERROR");

        //Suche eine Pflanze in der Liste und gebe deren Array Nummer zurück
        uint8_t SearchPlant(String name_rmv = "ERROR");

        //Bewässere alle Pflanzen 
        void Water_ALL();

        //Führe die Überprüfung der Pflanzenliste jede Millisekunde aus 
        //void MilliSec_Loop();
        void Sec_Loop(DateTime CurrentTime);

        void periodic_function(DateTime CurrentTime);
};

//PUMP------------------------------------------------------------------------------------------------------------
class Pump{
    public:
        static uint8_t Valve1_Port;         //Ausgangsport Ventil1
        static uint8_t Valve2_Port;         //Ausgangsport Ventil2
        static uint8_t Valve3_Port;         //Ausgangsport Ventil3
        static uint8_t Output_Port;         //Ausgangsport Pumpe
        static uint8_t LED_Pump;            //Ausgangsport LED_Pumpe

        static uint8_t LED_Number_Blinks;   //Wie oft soll die LED Blinken
        static uint8_t LED_Blinktime;       //Wie schnell soll die LED Blinken
        static bool LED_Pump_Condition;     //Zustand LED (HIGH oder LOW)
        static bool LED_deactivated_by_Pump;//Merkt sich ob LED durch Pumpe oder anderweitig deaktiviert wurde

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
        static uint8_t Printcounter;
        static void ControlTask();

        //Steuert die Hardware an 
        static void OutputControl();

        //Wird millisekündlich ausgeführt 
        static void Activation(DateTime CurrentTime_input, Plantlist *Pflanzenliste);

        static void LED_Blink();
};


class PLANT_EEPROM{
    public:
        static void Setup_and_Load(Plantlist *Pflanzenliste);
        static void Save(Plantlist Pflanzenliste);
};


class JSON_CONVERTER{
    public:
        static StaticJsonDocument<2048> JSON_Data_BT;
        static StaticJsonDocument<2048> JSON_Data_ESPNOW;
        static void Convert_JSON_TO_DATA_BT(Plantlist * Pflanzenliste, RTC_DS1307 *rtc_fnc, DateTime CurrentTime);
        static void Convert_JSON_TO_DATA_ESPNOW(Plantlist * Pflanzenliste, DateTime CurrentTime);
        static bool NEW_Bluetoothdata;
        static bool NEW_ESPNOW_Data;
        static bool RangeTest_STATUS;
        static String MAC_from_this_Device;
        static String Master_Call();
        static String RangeTest();
};


class Kalibration_Mode{
    public:
        static uint16_t Savety_Counter;
        static uint8_t Active;
        static uint8_t Valve;
        static Plantlist Kalibration_Pt_List;
        static Plantlist tmp_Plantlist;
        static void Kalibration_Loop(Plantlist *CurrPlantlist, DateTime CurrTime);
};
#endif