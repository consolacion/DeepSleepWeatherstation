/*
   Connection between RST and D0
   DHT on D2
   BMP sensor on I2C
*/
//WiFi
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>


// MQTT configuration
//
#include <PubSubClient.h>
WiFiClient wclient;
PubSubClient MQTTclient(wclient, "192.168.0.183");// < = replace by YOUR broker address


#include <SPI.h>  //only if you'd want to use the SPI of the BMP
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"
#include "config.h"

#define DHTTYPE DHT22
#define DHTPIN D5
#define SLEEPPIN D6 //aanbrengen pull up
// D1 en D2, (GPIO5 and GPIO4)  are SCL resp SDA  and used by the BMP280
#define FORCE_DEEPSLEEP //comment out when you like to use battery level dependent sleep


// OTA configuration
//
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* host = HOST;
// function prototypes required by Arduino IDE 1.6.7
void setupOTA(void);

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp280;

unsigned int batt;
double battV;
byte count = 0;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Setup Sketch...");
  dht.begin();

  Wire.begin();
  if (!bmp280.begin(0x76)) {
    Serial.println("no BMP280 detected");
    delay(1000);
  }

  setupWiFi();
  setupOTA();

  pinMode(SLEEPPIN, INPUT_PULLUP);
  Serial.print("Deep Sleep = ");
  Serial.println((digitalRead(SLEEPPIN) == LOW) ? "ON" : "OFF");
}

void loop(void) {
  ArduinoOTA.handle();
  batt = analogRead(A0);
  battV = batt * (4.2 / 1023);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //delay(2000);
  // float h = dht.readHumidity();
  //float t = dht.readTemperature();
  Serial.println("MQTT start");
  if (!MQTTclient.connected())
  {
    // connect if not already connected
    MQTTclient.connect("arduinoClient");
  }
  //Prevent useless DHT readings
  if (!isnan(h) || !isnan(t)) {
    MQTTclient.publish("garden/dht22/temp", String(t));
    MQTTclient.publish("garden/dht22/humidity", String(h));
  }
  MQTTclient.publish("garden/batt", String(battV));
  MQTTclient.publish("garden/battRaw", String(batt));
  MQTTclient.publish("garden/resetReason", ESP.getResetReason());
  MQTTclient.publish("garden/bmp280/temp", String(bmp280.readTemperature()));
  MQTTclient.publish("garden/bmp280/pressure", String(bmp280.readPressure() / 100));
  MQTTclient.publish("garden/sleep", (digitalRead(SLEEPPIN) == LOW) ? "ON" : "OFF");
  MQTTclient.disconnect();
  Serial.println("MQTT sent");
  delay(500); // time to allow for MQTT to be finished before going to sleep

#ifndef FORCE_DEEPSLEEP
  delay(30000);
#endif

  if (digitalRead(SLEEPPIN) == LOW) {
#ifdef FORCE_DEEPSLEEP
    Serial.println("Force deepsleep 15min!");
    ESP.deepSleep(15 * 60 * 1000000); // time in uSec  -> 15 min
    delay(100);
#endif

    //or handle deep sleep depending on battV
    if (battV < 3.3) {
      Serial.print("battery based sleep 30s");
      ESP.deepSleep(30 * 1000000); //30 sec
      delay(100);
    } else if (battV < 4.0) {
      Serial.print("battery sleep 2s");
      ESP.deepSleep(2 * 1000000); //2 sec
      delay(100);
    }
  } 
}



void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  //in case you want to send commands to the device
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}


