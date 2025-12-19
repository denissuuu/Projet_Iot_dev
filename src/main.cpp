#include <Arduino.h>           // Base Arduino   // SES besoins : timer, variables globales
#include "display_oled.h"
#include "pairing.h"
#include "wifi_access.h"


// WiFi Manager
// Reset WiFi Params
// Appairage
// MQTT
// DHT / OLED / MOVEMENT


void setup() {
    
  #if defined(DEBUG)
  Serial.begin(DEBUG_SPEED);
  #endif
  Serial.begin(115200);
  randomSeed(analogRead(0));  // Initialise le générateur avec bruit analogique

  generate_code();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
#if defined(D_DHT)
  // Read from sensors
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Ensure valid readings
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(5000);
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");

  // Publish sensor data to MQTT broker
  Serial.print("Temperature in Celsius:");
  Serial.println(String(temperature).c_str());
  client.publish(temperature_topic, String(temperature).c_str(), true);

  Serial.print("Humidity:");
  Serial.println(String(humidity).c_str());
  client.publish(Humidity_topic, String(humidity).c_str(), true);

  // Relay control with MQTT publish
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Toggle relay state
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    // Publish relay state
    String relayStatus = relayState ? "ON" : "OFF";
    Serial.print("Relay status: ");
    Serial.println(relayStatus);
    client.publish(relay_topic, relayStatus.c_str(), true);
  }


