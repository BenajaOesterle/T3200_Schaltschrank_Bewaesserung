#include <Arduino.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

#include "Plant.h"
#include "GlobalHeader.h"

TaskHandle_t Task1;
BluetoothSerial SerialBT;                                                     
StaticJsonDocument<2048> tmp_JSON_Data; 



#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run 'make menuconfig' to and enable it
#endif


void codeForTask1(void * parameter)
{
  //Einmalige Ausführung vergleichbar zum "void Setup()" auf Core 0
  SerialBT.begin("ESP32_Bewaesserung");                                           
  Serial.println("Bluetooth is started now you can pair your Device");  

  //Endlosschleife für den Empfang von Bluetoothdaten
  for(;;)
  {
    //Wenn Bluetoothdaten verfügbar sind, versuche diese auszuwerten
    if(SerialBT.available())                                                      
    {
      //Bluetoothdaten als JSON-Datei interpretieren. Falls nicht möglich setzte error
      DeserializationError error = deserializeJson(JSON_CONVERTER::JSON_Data_BT, SerialBT);

      //Wenn die Daten neu sind und noch nicht ausgewert, setzte "NEW_Bluetoothdata"
      if(!(tmp_JSON_Data == JSON_CONVERTER::JSON_Data_BT))
      {
        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        }
        else{
          Serial.print("deserializeJson() successed: ");
          JSON_CONVERTER::NEW_Bluetoothdata = true;

          //Gebe die empfangenen Daten auf dem Serial Monitor aus
          serializeJsonPretty(JSON_CONVERTER::JSON_Data_BT, Serial);
          Serial.println();
          serializeJson(JSON_CONVERTER::JSON_Data_BT, Serial);
        }
        //Empfangsbestätigung für den Sender
        SerialBT.println("Ready to send next");                 
      }
    }
    //Pausiere den Task für 60 Ticks um Hintergrundtasks Zeit zur ausführung zu geben
    vTaskDelay(60);
  }
}


void Tasksetup1()
{
  xTaskCreatePinnedToCore
  (
    codeForTask1,   //Funktion die Aufgerufen werden soll
    "Task1",        //Codewort des Tasks
    10000,          //Speicherreservierung auf dem Stack
    NULL,           //Task Input Parameter
    1,              //Priorität des Tasks 
    &Task1,         //Task Handle
    1               //CPU Core (0 ist der Standardcore)
  );
}

