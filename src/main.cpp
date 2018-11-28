/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "PIANO9"
#define WLAN_PASS       "Cipollina2002"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME "AndyMilly"
#define AIO_KEY "3648602c63ef49ad843ea6405f133ec7"

/********************************* MQTT Setup ********************************/
// Create an ESP8266 WiFiClient 
WiFiClient client;            // Not secure
// WiFiClientSecure client;   // SSL

// Setup WiFi client with login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
// NOTICE: MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish DataChannel = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/MD_Data");
Adafruit_MQTT_Subscribe StatusChannel = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/MD_Status");

/****************************** Support Variabled ***************************************/
uint8_t detection = 0;
uint8_t status = 1;

/************************ Function prototypes ************************************/
void MQTT_connect();

/********************************** Setup ****************************************/
void setup() 
{

    // Serial setup and init
    Serial.begin(9600);
    delay(10);

    // Wifi Connection
    Serial.println(); Serial.println(); Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(); Serial.println("WiFi connected");
    Serial.println("IP address: "); Serial.println(WiFi.localIP());

    // Setup MQTT subscription for onoff feed.
    mqtt.subscribe(&StatusChannel);

    // Setup random 
    randomSeed(analogRead(0));
}

void loop() 
{
    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    MQTT_connect();

    // Reaction phase
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000)))
    {
        if (subscription == &StatusChannel) 
        {
            status = StatusChannel.lastread[0];
            Serial.print(F("Got: "));
            Serial.println((char *)StatusChannel.lastread);
        }
    }

    // Detection phase!
    if (random(0, 10) > 6)
        detection = 1;
    else
        detection = 0;

    // Publication phase
    if (status > 0)
    {
        Serial.print(detection);
        Serial.print("...");
        if (! DataChannel.publish(detection))
            Serial.println(F("Failed"));
        else
            Serial.println(F("OK!"));
    }
    else
    {
        Serial.println("Offline Status");   
    }
    
    delay(5000);

    // ping the server to keep the mqtt connection alive
    // NOT required if you are publishing once every KEEPALIVE seconds
    /*
    if(! mqtt.ping()) {
        mqtt.disconnect();
    }
    */
}


void MQTT_connect() 
{
    int8_t ret;

    // Stop if already connected.
    if (mqtt.connected())
        return;

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) 
    { 
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000);  // wait 5 seconds
        retries--;
        if (retries == 0)
         while(1);   // basically die and wait for WDT to reset me
    }
    Serial.println("MQTT Connected!");
}
