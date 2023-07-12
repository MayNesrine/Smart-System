
// /*
//    Made By May Nesrine
//    Date 05-2023
//    Project SmartUsine
//    Read Sensor and publish_subscribe Data by the MQTT to NodeRed and Send it to firebase
// */
//Libraries
#define DEBUG
#include "DebugUtils.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#else
#include <WiFi.h>
#include <FirebaseESP32.h>
#endif
#include "DHT.h"
#include <Adafruit_Sensor.h>  //Adafruit Unified Sensor adapter witH DHT11 ESP8266
#include <PubSubClient.h>
//#include <ArduinoJson.h>
#include <DFRobot_MAX31855.h>
#include <Wire.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Définition de constants
// Definition of constants
#define LIGHT_SENSOR_PIN 34  // ESP32 pin GIOP36 (ADC0)
#define VIBPIN 33
#define DHTPIN 4       // Le pin de connexion du capteur
#define DHTTYPE DHT11  // Le type de capteur DHT 11
#define Motor 5
#define Ventilateur 15

float h, b1;               // pour stocké la valeur de l'humidité
float t, A1;               // pour stocké la valeur de la température
float temp, A2;            // pour stocké la valeur de la température de Moteur
int LIGHTValue, VibValue;  // pour stocké la valeur de la lumiére

// les topic pour la température et de l'humidité
// the topic for temperature and humidity

#define topic_C "Sensor/Data"
#define topic_M "Sys/Motor"

// instance de la classe DHT
// instance of the DHT class
DHT dht(DHTPIN, DHTTYPE);
// instance of the DFRobot_MAX31855 sensor class
DFRobot_MAX31855 max31855(&Wire, 0x10);

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* mqtt_server = "BROKER_ADRESS";  //or IP adress

/* Define the API Key */
#define API_KEY "API_KEY DATABASE_Firebase"

/*  Define the RTDB URL */
#define DATABASE_URL "RTDB_URL_Firebase"
//Define Firebase Data object
FirebaseData fbdo;
FirebaseConfig config;
const char* ssid     = "SSID_WIFI";
const char* password = "PASSWORD_WIFI";
void setup() {

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
#endif
  dht.begin();
  delay(1000);
  max31855.begin();
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(Motor, OUTPUT);
  pinMode(Ventilateur, OUTPUT);
  delay(1000);
 
  //check wifi communication
  DEBUG_PRINTLN("check_wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    DEBUG_PRINTLN(".");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  delay(1000);
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);

  delay(1500);

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(DATABASE_URL, API_KEY);

  Firebase.setDoubleDigits(5);
}

void callback(char* topic, byte* payload, unsigned int length) {

  String setAction;
  DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");

  for (int i = 0; i < length; i++) {
    DEBUG_PRINT((char)payload[i]);
    setAction += (char)payload[i];
  }
  //Rcv_Data[i] = 0;  // Null termination
  DEBUG_PRINTLN();
  if (String(topic) == topic_M) {
    DEBUG_PRINTLN("Changing output to ");
    if (setAction == "on") {
      DEBUG_PRINTLN("on");
      digitalWrite(Motor, 1);
    } else if (setAction == "off") {
      DEBUG_PRINTLN("off");
      digitalWrite(Motor, 0);
    } else {
      DEBUG_PRINTLN("off");
      digitalWrite(Motor, 0);
    }
    
  }
}
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect("oha")) {
      DEBUG_PRINTLN("connected");
      //mqtt.subscribe(topic_M);
      // Once connected, publish an announcement...
    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(mqtt.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (mqtt.connected()) {
    mqtt.subscribe(topic_M);
    pubCapteur();
    mqtt.loop();
  } else {
    reconnect();
  }
  SetFirebase();
}
/*Publish dataSensor in MQTT*/
void pubCapteur() {
  ReadCapteur();
  String Data = "{\"Temp_Motor\":" + String(temp) + ",\"temp\":" + String(t) + ",\"humi\":" + String(h) + ",\"LIGHT\":" + String(LIGHTValue) + ",\"Vibration_Motor\":" + String(VibValue) + ",\"action\":1}";
  mqtt.publish(topic_C, Data.c_str());
  delay(1000);
}
/*Set data in Firebase*/
void SetFirebase() {
  ReadCapteur();
  if (Firebase.ready()) {
    /*Firebase_temperatureSystem*/
    //setValue_RTDB
    Firebase.RTDB.setFloat(&fbdo, "/Sensor/Temperature_Sys", t);
    Firebase.RTDB.setFloat(&fbdo, "/Sensor/humidity_Sys", h);
    Firebase.RTDB.setFloat(&fbdo, "/Sensor/Temperature_Motor", temp);
    Firebase.RTDB.setInt(&fbdo, "/Sensor/Vibration_Motor", VibValue);
    Firebase.RTDB.setInt(&fbdo, "/Sensor/Luminosity", LIGHTValue);
    delay(200);

    DEBUG_PRINTLN("------------------");
    DEBUG_PRINTLN();
    delay(1000);
    //setValue_RTDBstorage
    Firebase.RTDB.pushFloat(&fbdo, "/storage/Temperature_Sys", t);
    Firebase.RTDB.pushFloat(&fbdo, "/storage/humidity_Sys", h);
    Firebase.RTDB.pushFloat(&fbdo, "/storage/Temperature_Motor", temp);
    Firebase.RTDB.pushInt(&fbdo, "/storage/Vibration_Motor", VibValue);
    Firebase.RTDB.pushInt(&fbdo, "/storage/Luminosity", LIGHTValue);
  }
}
/*Read Sensor*/
void ReadCapteur() {
  /*Read max31855*/
  temp = max31855.readCelsius();
  DEBUG_PRINT("Temperature_Motor: ");
  DEBUG_PRINT(temp);
  DEBUG_PRINTLN(" °C");
  DEBUG_PRINTLN();
  if (temp >= 24) digitalWrite(Ventilateur, 1);
  else digitalWrite(Ventilateur, 0);

  /*Read Vib_SENSOR_LDR*/
  VibValue = analogRead(VIBPIN);
  DEBUG_PRINT("VIBValue = ");
  DEBUG_PRINTLN(VibValue);
  DEBUG_PRINTLN();

  /*Read LIGHT_SENSOR_LDR*/
  LIGHTValue = analogRead(LIGHT_SENSOR_PIN);
  DEBUG_PRINT("LDRValue = ");
  DEBUG_PRINTLN(LIGHTValue);
  DEBUG_PRINTLN();

  /*Read DHT11*/
 
  float newH = dht.readHumidity();
  if (isnan(newH)) {
    DEBUG_PRINTLN("Failed to read from DHT sensor!");
  } else {
    h = newH;
    DEBUG_PRINT("HumidityAmb: ");
    DEBUG_PRINT(h);
    DEBUG_PRINTLN("%");
  }
  float newT = dht.readTemperature();
  if (isnan(newT)) {
    DEBUG_PRINTLN("Failed to read from DHT sensor!");
  } else {
    t = newT;
    DEBUG_PRINT("TemperatureAmb: ");
    DEBUG_PRINT(t);
    DEBUG_PRINTLN(" °C");
    DEBUG_PRINTLN();
  }
  DEBUG_PRINTLN();
  delay(1000);
}
