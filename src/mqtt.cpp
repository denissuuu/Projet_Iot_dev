// #include <Arduino.h>
// #include <wifi_access.h>
// #include <PubSubClient.h>

// // MQTT Broker details
// #define mqtt_server  "broker.emqx.io" // check with ipconfig in cmd, choose IPv4 address

// // Pins
// #if defined(D_DHT)
// #define Humidity_topic  "ynov/rennes/damien/humidity"
// #define temperature_topic "ynov/rennes/damien/temperature"
// #define relay_topic "ynov/rennes/damien/status"
// #include <DHT.h>
// #define DHTPIN 23           // Pin where the DHT11 is connected
// #define DHTTYPE DHT11      // DHT11 sensor
// #define RELAY_PIN 26       // Pin where the relay is connected

// // DHT Sensor setup
// DHT dht(DHTPIN, DHTTYPE);

// bool relayState = false;
// unsigned long previousMillis = 0;
// const long interval = 2000; // 2 seconds on/off cycle
// #endif
// // WiFi and MQTT client
// WiFiClient espClient;
// PubSubClient client(espClient);


// void reconnect() {
//   while (!client.connected()) {
//     Serial.print("Tentative MQTT...");
//     // ✅ Remplace getChipId par getEfuseMac
//     String clientId = "ESP32-" + String(ESP.getEfuseMac(), HEX);

    
    
//     if (client.connect(clientId.c_str())) {
//       Serial.println("connecté!");
//       client.subscribe("ynov/tempera/");
//       client.publish("ynov/tempera/status", "online");
//     } else {
//       Serial.print("échec, rc="); Serial.print(client.state());
//       Serial.println(" retry in 5s");
//       delay(5000);
//     }
//   }
// }

// void setup() {
// #if defined(D_DHT) // Initialize the DHT sensor
//   dht.begin();
//   pinMode(RELAY_PIN, OUTPUT);
//   digitalWrite(RELAY_PIN, LOW);
// #endif
//   // Setup WiFi and MQTT
//   setup_wifi();
//   client.setServer(mqtt_server, 1883);
// }

// void loop() {
//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();
// #if defined(D_DHT)
//   // Read from sensors
//   float temperature = dht.readTemperature();
//   float humidity = dht.readHumidity();

//   // Ensure valid readings
//   if (isnan(temperature) || isnan(humidity)) {
//     Serial.println("Failed to read from DHT sensor!");
//     delay(5000);
//     return;
//   }

//   Serial.print("Humidity: ");
//   Serial.print(humidity);
//   Serial.print(" %\t");
//   Serial.print("Temperature: ");
//   Serial.print(temperature);
//   Serial.print(" *C ");

//   // Publish sensor data to MQTT broker
//   Serial.print("Temperature in Celsius:");
//   Serial.println(String(temperature).c_str());
//   client.publish(temperature_topic, String(temperature).c_str(), true);

//   Serial.print("Humidity:");
//   Serial.println(String(humidity).c_str());
//   client.publish(Humidity_topic, String(humidity).c_str(), true);

//   // Relay control with MQTT publish
//   unsigned long currentMillis = millis();
//   if (currentMillis - previousMillis >= interval) {
//     previousMillis = currentMillis;
//     // Toggle relay state
//     relayState = !relayState;
//     digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
//     // Publish relay state
//     String relayStatus = relayState ? "ON" : "OFF";
//     Serial.print("Relay status: ");
//     Serial.println(relayStatus);
//     client.publish(relay_topic, relayStatus.c_str(), true);
//   }
// #endif
//   // Delay before next reading
//   delay(5000); // Publish every 5 seconds
// }