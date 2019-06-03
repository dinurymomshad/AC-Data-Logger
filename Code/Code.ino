#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "STELLAR"
#define WLAN_PASS       "stellarBD"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Momshad"
#define AIO_KEY         "a4cd964d9a2d411d844a18028399de5e"

/****************************** Global State ************************************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/************************ Millis() Variable *********************************/
int waitingPeriod = 10000;
int currentTime = 0;
/*************************** Sketch Code ************************************/

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(A0, INPUT);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  //  mqtt.subscribe(&onoffbutton);
}

uint32_t x;
uint32_t prevX;

void loop() {

  MQTT_connect();

  //  Adafruit_MQTT_Subscribe *subscription;
  //  while ((subscription = mqtt.readSubscription(5000))) {
  //    if (subscription == &onoffbutton) {
  //      Serial.print(F("Got: "));
  //      Serial.println((char *)onoffbutton.lastread);
  //    }
  //  }

  //Check for peek value after every 10 second
  if (millis() > currentTime + waitingPeriod) {
    currentTime = millis();
    while (millis() - currentTime < 1000) { //Take data for one second
      x = analogRead(A0);
      Serial.println(x);
      if (x > prevX) { //Only track the max value
        prevX = x;
      }
    }
    currentTime = millis();
    Serial.print("...");
    Serial.println(prevX);
    if (! photocell.publish(prevX)) { //publish the max value
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
    prevX = 0;
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
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
}
