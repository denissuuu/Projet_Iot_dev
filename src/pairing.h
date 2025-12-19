#ifndef PAIRING_H
#define PAIRING_H
#include <iostream>
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display;
void timer(int code);
void display_code(int code);
int generate_code(int code);
#endif