<<<<<<< HEAD
=======
#include <Arduino.h>


// WiFi Manager
// Reset WiFi Params
// Appairage
// MQTT
// DHT / OLED / MOVEMENT

void setup() {
  #if defined(DEBUG)
  Serial.begin(DEBUG_SPEED);
  #endif
}

void loop() {

  #if DELAY_COUCOU == 1
  delay(5000); // Publish every 5 seconds
  #elif DELAY_COUCOU == 2
  delay(10000); // Publish every 10 seconds
  #endif
  #if defined (DEBUG)
  Serial.println("coucou");
  #endif
}
//coucou
>>>>>>> 64dea4e1de603d963d41fb0beec922b208480709
