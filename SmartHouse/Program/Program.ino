
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>
#include <SimpleTimer.h>
#else
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#endif

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
//#include <Wire.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Définition de constants
// Definition of constants

#define MVT_SENSOR_PIN 34
#define GAZ_SENSOR_PIN 32
#define DHTPIN 4       // Le pin de connexion du capteur
#define DHTTYPE DHT11  // Le type de capteur DHT 11
#define Led1 5
#define Led2 15
#define Led3 22
#define Led4 23

float h;     // pour stocké la valeur de l'humidité
float t;     // pour stocké la valeur de la température
bool mvtValue;  // pour stocké la valeur de la température de Moteur
int GazValue;  // pour stocké la valeur de la lumiére

// instance de la classe DHT
// instance of the DHT class
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);
SimpleTimer timer;
//config Blynk
#define BLYNK_TEMPLATE_ID "TMPL2s42lpVBg"
#define BLYNK_TEMPLATE_NAME "DHT11"
#define BLYNK_AUTH_TOKEN "TTapqcvoMexvYsRQtNA0Fw4Eg3rGdZ7W"
char auth[] = BLYNK_AUTH_TOKEN;

/* 2. Define the API Key */
#define API_KEY "9heLRWUTJlO3Y4wHLVRLRiswGoOkzj6CLUL0za6V"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://database-esp32-d9c10-default-rtdb.firebaseio.com/" 
//Define Firebase Data object
FirebaseData fbdo;
FirebaseConfig config;
char ssid[] = "sonore";
char pass[] = "sa3dounmay";
WebServer server(80);
//ESP8266WebServer server(80);
bool cmdBlynk = false;
void setup() {
  Serial.begin(115200);
  timer.setInterval(2000, loop);
  dht.begin();
  delay(1000);
  pinMode(DHTPIN, INPUT);
  //  pinMode(MVT_SENSOR_PIN, INPUT);
  //  pinMode( GAZ_SENSOR_PIN, INPUT);

  pinMode(Led1, OUTPUT);
  pinMode(Led2, OUTPUT);
  pinMode(Led3, OUTPUT);
  pinMode(Led4, OUTPUT);

  digitalWrite(Led1, LOW);
  digitalWrite(Led2, LOW);
  digitalWrite(Led3, LOW);
  digitalWrite(Led4, LOW);
  delay(1000);

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("My home");
  lcd.clear();
  delay(1000);
  //Define wifi connection
  Serial.println("check_wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  //check_wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(DATABASE_URL, API_KEY);

  Firebase.setDoubleDigits(5);
  Blynk.begin(auth, ssid, pass);

  Setup_ServerCtrl();
}
void loop() {
  ReadSensor();
  while (cmdBlynk) {
    Blynk.run();
    AppBlynk();
  }
  SetFirebase();
  server.handleClient();
}
/*Set data in Firebase*/
void SetFirebase() {
  if (Firebase.ready()) {
    //setValue_RTDB
    Firebase.RTDB.setFloat(&fbdo, "/Smart-House/Temperature", t);
    Firebase.RTDB.setFloat(&fbdo, "/Smart-House/humidity", h);
    Firebase.RTDB.setBool(&fbdo, "/Smart-House/Mouvement", mvtValue);
    Firebase.RTDB.setInt(&fbdo, "/Smart-House/GAZ", GazValue);
    delay(200);
    Serial.println();
    delay(1000);

  }
  else {
    cmdBlynk = true;
    Serial.println("AppBlynk");

  }
}

void ReadSensor() {

  /*Read DHT11*/
  float newT = dht.readTemperature();
  if (isnan(newT)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    t = newT;
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" °C");
    //print in LCD
    lcd.setCursor(2, 0);  //Set cursor to character 2 on line 0
    lcd.print("temp=");
    lcd.print(t);
  }

  float newH = dht.readHumidity();
  if (isnan(newH)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    h = newH;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
    Serial.println();
    //print in LCD
    lcd.setCursor(2, 1);  //Set cursor to character 2 on line 0
    lcd.print("Humid=");
    lcd.print(h);
  }
  /*Read Gaz-Sensor*/
  int GazV = analogRead(GAZ_SENSOR_PIN);
  GazValue=GazV/100;
  Serial.print("Gaz_Value: ");
  Serial.print(GazValue);
  Serial.println();
  /*Read Mouvement_SENSOR*/
  mvtValue = digitalRead(MVT_SENSOR_PIN);
  Serial.print("Mouvement = ");
  Serial.println(mvtValue);
  Serial.println();
  delay(1000);
}
void AppBlynk() {
  Blynk.virtualWrite(V0, GazValue);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V3, mvtValue);
  delay(2000);
  cmdBlynk = false;
  Serial.println();
}
void Setup_ServerCtrl() {
  //  server.on("/", []() {
  ////      server.send(200, "text/html", webpage);
  // });
  /*Controle-eclairage*/
  server.on("/Led1ON", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led1, HIGH);
    Serial.println("/Led1ON");

    delay(1000);
  });

  server.on("/Led1OFF", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led1, LOW);
    Serial.println("/Led1OFF");
    delay(1000);
  });
  server.on("/Led2ON", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led2, HIGH);
    Serial.println("/Led2ON");
    delay(1000);
  });

  server.on("/Led2OFF", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led2, LOW);
    Serial.println("/Led2OFF");
    delay(1000);
  });
  server.on("/Led3ON", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led3, HIGH);
    delay(1000);
  });

  server.on("/Led3OFF", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led3, LOW);
    delay(1000);
  });
  server.on("/Led4ON", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led4, HIGH);
    delay(1000);
  });

  server.on("/Led4OFF", []() {
    //server.send(200, "text/html", webpage);
    digitalWrite(Led4, LOW);
    delay(1000);
  });
  /*Controle-portes*/
  server.on("/P1ON", []() {
    Serial.println("Porte principale ouverte");
    delay(1000);
  });

  server.on("/P1OFF", []() {
    Serial.println("Porte principale fermée");
    delay(1000);
  });
  server.on("/P2ON", []() {
    Serial.println("Porte garage ouverte");
    delay(1000);
  });

  server.on("/P2OFF", []() {
    Serial.println("Porte garage fermée");
    delay(1000);
  });
/*Controle fenetre*/
server.on("/F1ON", []() {
    Serial.println("Fenetre cuisine ouverte");
    delay(1000);
  });

  server.on("/F1OFF", []() {
    Serial.println("Fenetre cuisine fermée");
    delay(1000);
  });
  server.on("/F2ON", []() {
    Serial.println("Fenetre salon ouverte");
    delay(1000);
  });

  server.on("/F2OFF", []() {
    Serial.println("Fenetre salon fermée");
    delay(1000);
  });
  server.on("/F3ON", []() {
    Serial.println("Fenetre Chambre enfant ouverte");
    delay(1000);
  });

  server.on("/F3OFF", []() {
    Serial.println("Fenetre Chambre enfant fermée");
    delay(1000);
  });
  /*Controle Store*/
    server.on("/S1ON", []() {
    Serial.println("Store Salon ouverte");
    delay(1000);
  });

  server.on("/S1OFF", []() {
    Serial.println("Store Salon fermée");
    delay(1000);
  });
    server.on("/S2ON", []() {
    Serial.println("Store Cuisine ouverte");
    delay(1000);
  });

  server.on("/S2OFF", []() {
    Serial.println("Store Cuisine fermée");
    delay(1000);
  });
    server.on("/S3ON", []() {
    Serial.println("Store Chambre enfant ouverte");
    delay(1000);
  });

  server.on("/S3OFF", []() {
    Serial.println("Store Chambre enfant fermée");
    delay(1000);
  });
    server.on("/S4ON", []() {
    Serial.println("Store Chambre Parent ouverte");
    delay(1000);
  });

  server.on("/S4OFF", []() {
    Serial.println("Store Chambre Parent fermée");
    delay(1000);
  });
  server.begin();
  Serial.println("HTTP server started");
}
