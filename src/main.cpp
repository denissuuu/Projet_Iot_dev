#include <Arduino.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "display_oled.h"
#include "pairing.h"
#include "wifi_access.h"
#include "PIR.h"
#include "BootCounter.h"

// Configuration MQTT
const char* mqtt_server = "broker.emqx.io";
WiFiClient espClient;
PubSubClient client(espClient);

// Timers et Variables
unsigned long dernierEnvoiCode = 0;
unsigned long prevDHTMillis = 0;
const long intervalleCode = 60000;
const long intervalleData = 10000;

String codeActuel = "";
float lastT = 0.0, lastH = 0.0;
char lastP = 'N'; 
BootCounter bc; 

#if defined(D_DHT)
#include <DHT.h>
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define RELAY_PIN 26
#endif

// Fonction de reconnexion MQTT
void reconnect() {
    while (!client.connected()) {
        Serial.print("MQTT Connexion...");
        String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("OK");
            client.publish("ynov/status", "online");
        } else {
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    bc.begin(); // Sécurité boot
    if (bc.shouldResetWifi()) {
        bc.resetEverything(); 
        WiFiManager wm;
        wm.resetSettings();   
        ESP.restart();        
    }
    randomSeed(analogRead(0));
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    init_PIR(); 
    #if defined(D_DHT)
    dht.begin();
    pinMode(RELAY_PIN, OUTPUT);
    #endif
    setup_wifi();
    bc.resetBootCounter();
    client.setServer(mqtt_server, 1883);
}

void loop() {
    if (!client.connected()) reconnect();
    client.loop();

    unsigned long maintenant = millis();
    String macID = String((uint32_t)ESP.getEfuseMac(), HEX);

    // 1. Envoi Code Pairing (60s)
    if (maintenant - dernierEnvoiCode >= intervalleCode || codeActuel == "") {
        dernierEnvoiCode = maintenant;
        codeActuel = generate_code();
        display_code(codeActuel);
        String tPairing = "ynov/appairage/" + macID + "/code";
        client.publish(tPairing.c_str(), codeActuel.c_str(), true);
    }

    // 2. Détection Mouvement (PIR sur PIN 27)
    if (motionDetected) {
        motionDetected = false;
        lastP = 'Y';
        int count = EEPROM.read(EEPROM_ADDR);
        count++;
        EEPROM.write(EEPROM_ADDR, count);
        EEPROM.commit();
        if (count >= RESET_THRESHOLD) {
            WiFiManager wm;
            wm.resetSettings();
            EEPROM.write(EEPROM_ADDR, 0);
            EEPROM.commit();
            ESP.restart();
        }
    }

    // 3. Envoi Django (10s)
    if (maintenant - prevDHTMillis >= intervalleData) {
        prevDHTMillis = maintenant;
        #if defined(D_DHT)
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t)) lastT = t;
        if (!isnan(h)) lastH = h;
        #endif

        String dataStr = String(lastT, 1) + "°C, " + String(lastH, 1) + "%, " + lastP;
        String tDjango = "ynov/home/" + macID + "/sensors";
        String payloadJSON = "{\"msg\": \"" + dataStr + "\"}";

        client.publish(tDjango.c_str(), payloadJSON.c_str());
        Serial.println("Vers Django: " + payloadJSON);
        lastP = 'N'; 
    }
}