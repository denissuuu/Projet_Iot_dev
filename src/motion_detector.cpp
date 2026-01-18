#include "PIR.h"
#include <EEPROM.h>

volatile bool motionDetected = false;

void IRAM_ATTR handleMotion() {
    motionDetected = true;
}

void init_PIR() {
    EEPROM.begin(512);
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), handleMotion, RISING);
    
    int count = EEPROM.read(EEPROM_ADDR);
    Serial.print("PIR Initialisé. Compteur actuel en EEPROM: ");
    Serial.println(count);
}