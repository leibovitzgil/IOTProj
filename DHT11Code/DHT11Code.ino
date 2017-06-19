#include <SPI.h>  
#include "RF24.h" 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const int ldrPin = A0;  //the number of the LDR pin

#include <SPI.h>  
#include "RF24.h"


#define DHTPIN            7
#define DHTTYPE           DHT11     // DHT 11

RF24 myRadio (9, 10);
byte addresses[][6] = {"0"};

struct package
{
  int id=1;
  float temperature = 18.3;
  char  text[100] = "Text to be transmitted";
};


typedef struct package Package;
Package data;
DHT_Unified dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);
  myRadio.begin();  
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MIN);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openWritingPipe( addresses[0]);
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
   // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
}

void loop()
{

   sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    data.temperature = event.temperature;
    Serial.println(" *C");
  }
  
  
  myRadio.write(&data, sizeof(data)); 

  Serial.print("\nPackage:");
  Serial.print(data.id);
  Serial.print("\n");
  Serial.println(data.temperature);
  Serial.println(data.text);
  data.id = data.id + 1;

  delay(10000);
  //data.temperature = data.temperature+0.1;
  
//  int ts=micros();
//  while(micros()-ts>10000){
//    Serial.print("Reaching Loop");
//  }
  

}
