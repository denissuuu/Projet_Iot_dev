#ifndef PIR_H
#define PIR_H

#include <Arduino.h>

#define PIR_PIN 27  // CHANGÉ de 21 à 27 pour éviter le conflit avec l'OLED
#define EEPROM_ADDR 20
#define RESET_THRESHOLD 5

extern volatile bool motionDetected;

void IRAM_ATTR handleMotion();
void init_PIR(); // Nouvelle fonction pour le setup

#endif