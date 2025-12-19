#include <pairing.h>


// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 30000;  // interval at which to blink (milliseconds)

#include <display_oled.h>

#include <Adafruit_SSD1306.h> //OLED
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

unsigned long messageDebut = 0;
bool messageIsActif = false;
const unsigned long DUREE_MESSAGE = 60000;  
volatile bool declencherAffichage = false;  



int generate_code(int code) {

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
    String code = "";  // Chaîne pour stocker les 6 chiffres
    for (int i = 0; i < 6; i++) {
        int chiffre = random(0, 10);  // 0 à 9 par chiffre
        code += String(chiffre);
    }
    Serial.println("Code: " + code);  // Affiche ex: "483920"
    }
    return code;
}

void timer(int code) {
 unsigned long currentMillis = millis();
 int previousMillis = currentMillis;
    if (currentMillis - previousMillis >= 60000) {
    generate_code(code);
    display_code(code);
    previousMillis = currentMillis;
    }
}

void display_code(int code) {
    display.clearDisplay();
    display.setTextSize(2);      
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);     
    display.print(code);
    display.display();
}







