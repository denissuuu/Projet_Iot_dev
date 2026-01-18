#include "pairing.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

String generate_code() {
    String code = ""; 
    for (int i = 0; i < 6; i++) {
        code += String(random(0, 10));
    }
    return code;
}

void display_code(String code) {
    display.clearDisplay();
    display.setTextSize(2);      
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);     
    display.print(code);
    display.display();
}