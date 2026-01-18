// #include <EEPROM.h>
// #include "PIR.h"

// #define EEPROM_ADDR 0
// #define RESET_THRESHOLD 5

// void setup() {
//   Serial.begin(115200);
//   EEPROM.begin(512);

//   pinMode(PIR_PIN, INPUT);
//   attachInterrupt(digitalPinToInterrupt(PIR_PIN), handleMotion, RISING);
  
//   int count = EEPROM.read(EEPROM_ADDR);
//   Serial.print("Current count: ");
//   Serial.println(count);
// }

// void loop() {
//   if (motionDetected) {
//     motionDetected = false;

//     int count = EEPROM.read(EEPROM_ADDR);
//     count++;
//     Serial.print("Motion detected count: ");
//     Serial.println(count);

//     if (count >= RESET_THRESHOLD) {
//       Serial.println("Reset threshold reached! Resetting WiFi settings...");
// }
//   }
// }
