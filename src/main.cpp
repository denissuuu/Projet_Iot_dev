#include <Arduino.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "display_oled.h"
#include "pairing.h"
#include "wifi_access.h"
#include "PIR.h"
#include "BootCounter.h"

// MQTT CONFIGURATION 
const char* mqtt_server = "broker.emqx.io";
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long dernierEnvoiCode = 0;
const long intervalleCode = 60000;
String codeActuel = "";

BootCounter bc; 


#if defined(D_DHT)
#include <DHT.h>
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define RELAY_PIN 26
bool relayState = false;
unsigned long prevDHTMillis = 0;
#endif

void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentative MQTT...");
        String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connecté!");
            client.publish("ynov/status", "online");
        } else {
            Serial.print("échec, rc=");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // reset boot counter
    bc.begin();
    Serial.print("Nombre de boots : ");
    Serial.println(bc.getCount());

    // boot reset 
    if (bc.shouldResetWifi()) {
        Serial.println("Reset déclenché par 5 boots consécutifs !");
        bc.resetEverything(); 
        WiFiManager wm;
        wm.resetSettings();   
        ESP.restart();        
    }

    randomSeed(analogRead(0));
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    
    init_PIR(); 

    setup_wifi();
    
    bc.resetBootCounter();

    client.setServer(mqtt_server, 1883);

    #if defined(D_DHT)
    dht.begin();
    pinMode(RELAY_PIN, OUTPUT);
    #endif

    Serial.println("Système prêt !");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long maintenant = millis();

    // 1. LOGIQUE PAIRING
    if (maintenant - dernierEnvoiCode >= intervalleCode || codeActuel == "") {
        dernierEnvoiCode = maintenant;
        codeActuel = generate_code();
        display_code(codeActuel);
        String topic = "ynov/appairage/" + String((uint32_t)ESP.getEfuseMac(), HEX) + "/code";
        client.publish(topic.c_str(), codeActuel.c_str(), true);
    }

    // 2. LOGIQUE PIR (Mouvement)
    if (motionDetected) {
        motionDetected = false;
        int count = EEPROM.read(EEPROM_ADDR);
        count++;
        EEPROM.write(EEPROM_ADDR, count);
        EEPROM.commit();

        Serial.print("Mouvement détecté ! Compteur: ");
        Serial.println(count);
        
        // Envoi MQTT du mouvement
        String mTopic = "ynov/rennes/damien/motion";
        client.publish(mTopic.c_str(), String(count).c_str());

        if (count >= RESET_THRESHOLD) {
            Serial.println("Seuil atteint ! Reset WiFi...");
            WiFiManager wm;
            wm.resetSettings();
            EEPROM.write(EEPROM_ADDR, 0); // Reset du compteur
            EEPROM.commit();
            ESP.restart();
        }
    }

    // 3. LOGIQUE DHT
    #if defined(D_DHT)
    if (maintenant - prevDHTMillis >= 5000) {
        prevDHTMillis = maintenant;
        float t = dht.readTemperature();
        if (!isnan(t)) {
            client.publish("ynov/rennes/damien/temperature", String(t).c_str());
        }
    }
    #endif
}