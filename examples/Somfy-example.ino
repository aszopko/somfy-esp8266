/**
 * 
 *  Super easy to use Somfy RTS blind controller library example
 *  MQTT subscription example included
 *  
 */

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Somfy.h> 

#define AIO_SERVER      "192.168.000.000" //IP of your mqtt server
#define AIO_SERVERPORT  1883              //port of mqtt server - use 8883 for SSL
#define AIO_USERNAME    "mqtt_username"
#define AIO_KEY         "mqtt_password"
#define RF_SENDER D2    //this is where you connected the RF sender data pin

const bool TEST = 0;
const int MAXTRIES = 5; // how many times should we try to reconnent to host
const char* ssid     = "WIFI_SSID";
const char* password = "WIFI password";

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe mqttSomfy = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/garage/blinds");
Adafruit_MQTT_Subscribe *subscription;

Somfy somfy(RF_SENDER, 0x134452); //initial remote id - change this one,  it will be incremented for remotes added afterwards

void setup() {

  somfy.AddRemote("office");
//usage: send an MQTT message "[name of blind]+somfyCommand[U|D|S|P]", e.g. "officeU"
//you can add more remotes
  somfy.AddRemote("gym");
  somfy.AddRemote("storage");

  somfy.Setup();

  Serial.begin(115200);
  Serial.flush();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  ReconnectToWiFi("setup");  
    
  Serial.println("");
  mqtt.subscribe(&mqttSomfy);
  logger("WiFi connected :)");  
  logger("IP address: " + WiFi.localIP().toString());

  logger("Setup completed");
}

void loop() {
    MQTT_connect(); 
    while ((subscription = mqtt.readSubscription(1000))) {
      if (subscription == &mqttSomfy) {
        logger("Message in blinds topic: " + String((char*)mqttSomfy.lastread));
        logger(somfy.ProcessMessageAndExecuteCommand((char*)mqttSomfy.lastread));
        }        
      }
}

void ReconnectToWiFi(String task)
{
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  int8_t ret;
  uint8_t retries = 3;

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
   logger("MQTT Connected!");
}

void logger(String msg){
  if (TEST) {
    Serial.println(msg);
  }
}
