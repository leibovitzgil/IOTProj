/*
Sending data every 20 seconds by using Thingspeak HTTP API
*/

#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

WiFiClient client;

/*
  *****************************************************************************************
  **** Visit https://www.thingspeak.com to sign up for a free account and create
  **** a channel.  The video tutorial http://community.thingspeak.com/tutorials/thingspeak-channels/ 
  **** has more information. You need to change this to your channel, and your write API key
  **** IF YOU SHARE YOUR CODE WITH OTHERS, MAKE SURE YOU REMOVE YOUR WRITE API KEY!!
  *****************************************************************************************/
unsigned long myChannelNumber = 280464;
const char * myWriteAPIKey = "2TQPS1QSYXSLCB8C";

const char* ssid     = "AndroidAP";
const char* password = "xbjx1325";

void setup() {
  Serial.begin(115200);
  //Bridge.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
ThingSpeak.begin(client);
}

void loop() {
  float voltage = 10;
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  ThingSpeak.writeField(myChannelNumber, 1, voltage, myWriteAPIKey);
  delay(20000); // ThingSpeak will only accept updates every 15 seconds.
}
