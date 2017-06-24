/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <DHT.h> 
#include <string>

#define DHTPIN D5  
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
String temperature, humidity, light;
//int temperature, humidity;

WiFiClient tsclient;

const char* ssid     = "AndroidAP";
const char* password = "xbjx1325";
//thingspeak keys
unsigned long myChannelNumber = 280464;
const char * myWriteAPIKey = "2TQPS1QSYXSLCB8C";

const char* host = "192.168.43.110";
//const char* streamId   = "....................";
//const char* privateKey = "....................";

void setup() {
  Serial.begin(115200);
  dht.begin();  
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(tsclient);
}

int value = 0;

void loop() {
  delay(5000);
  ++value;
  
  //sensors data 
  temperature = dht.readTemperature();  
  humidity = dht.readHumidity(); 
  light = analogRead(A0);

  Serial.print("connecting to ");
  Serial.println(host);
  
  Serial.print("Temperature Value is :");  
  Serial.print(temperature);  
  Serial.println("C");  
  Serial.print("Humidity Value is :");  
  Serial.print(humidity);  
  Serial.println("%");
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 5000;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  /*
  // We now create a URI for the request
  String url = "/input/";
  url += streamId;
  url += "?private_key=";
  url += privateKey;
  url += "&value=";
  url += value;
  */
  //String url = "http://192.168.43.110:5000/obs?data={\"date\":\"33.7.2012\"}";
  //String url = "http://192.168.43.110:5000/obs?data=" + "{\"date\":\"33.7.2012\"," + "humidity:" + humidity + "," + "temperature:" + temperature + "}";
  //String url = "http://192.168.43.110:5000/obs?data=" + "{\"date\":\"33.7.2012\"" + "," + "humidity:" + humidity + "," + "temperature:" + temperature + "}";

  //String url = "http://192.168.43.110:5000/obs?data={\"date\":\"33.7.2012\"";

  // We now create a URI for the request
  String url = "http://192.168.43.110:5000/obs?data=";
  url += "{\"humidity\":";
  url += humidity;
  url += ",";
  url += "\"temperature\":";
  url += temperature;
  url += ",";
  url += "\"light\":";
  url += light;
  url += "}";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
                         
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
   
  
  Serial.println();
  Serial.println("closing connection");

  Serial.println("uploading data to thingspeak..");

   
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.

   ThingSpeak.setField(1,humidity);
   ThingSpeak.setField(2,temperature);
   ThingSpeak.setField(3,light);


   // Write the fields that you've set all at once.
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); 


  Serial.println("done uploading data to thingspeak..!");

  delay(20000);
}

