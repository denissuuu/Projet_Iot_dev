#include <Arduino.h>
#include <PubSubClient.h>
#include "display_oled.h"
#include "pairing.h"
#include "wifi_access.h"

// MQTT CONFIGURATION 
const char* mqtt_server = "broker.emqx.io";
WiFiClient espClient;
PubSubClient client(espClient);

// pairing timer variable
unsigned long dernierEnvoiCode = 0;
const long intervalleCode = 60000; // 1 minute
String codeActuel = "";

// dht variable
#if defined(D_DHT)
#include <DHT.h>
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define RELAY_PIN 26
bool relayState = false;
unsigned long prevDHTMillis = 0;
#endif

// reconnect function
void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentative MQTT...");
        // unique identifier
        String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connecté!");
            client.publish("ynov/status", "online");
        } else {
            Serial.print("échec, rc=");
            Serial.print(client.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(0));

    // wifi initialising
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println("Error : no oled screen found");
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("WiFi Config...");
    display.println("Connect to:");
    display.println("AutoConnectAP");
    display.display(); 

    setup_wifi();
    client.setServer(mqtt_server, 1883);

    #if defined(D_DHT)
    dht.begin();
    pinMode(RELAY_PIN, OUTPUT);
    #endif

    Serial.println("Système prêt !");
}

void loop() {
    // MQTT connexion
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long maintenant = millis();

    // pairing logic (Toutes les 60s)
    if (maintenant - dernierEnvoiCode >= intervalleCode || codeActuel == "") {
        dernierEnvoiCode = maintenant;
        
        // print/generation
        codeActuel = generate_code();
        display_code(codeActuel);

        // send to web via topic
        String macID = String((uint32_t)ESP.getEfuseMac(), HEX);
        String topic = "ynov/appairage/" + macID + "/code";
        client.publish(topic.c_str(), codeActuel.c_str(), true);
        
        Serial.println("Code envoyé sur : " + topic);
    }

    // 3. captor logic (DHT)
    #if defined(D_DHT)
    if (maintenant - prevDHTMillis >= 5000) { // Lecture toutes les 5s
        prevDHTMillis = maintenant;
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t) && !isnan(h)) {
            client.publish("ynov/rennes/damien/temperature", String(t).c_str());
            client.publish("ynov/rennes/damien/humidity", String(h).c_str());
            
            // Logique Relais
            relayState = !relayState;
            digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
            client.publish("ynov/rennes/damien/status", relayState ? "ON" : "OFF");
        }
    }
    #endif
}