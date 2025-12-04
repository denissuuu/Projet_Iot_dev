#ifndef BOOT_COUNTER_H
#define BOOT_COUNTER_H

#include <EEPROM.h>

class BootCounter {
public:
    BootCounter();

    void begin();                // Initializes EEPROM and increments counter
    int getCount();              // Returns current boot count
    bool shouldResetWifi();      // True if 5 boots reached
    void resetBootCounter();     // Resets boot counter only
    void resetEverything();      // Resets counter + wifi params

private:
    const int EEPROM_SIZE = 64;
    const int BOOT_COUNT_ADDR = 0;
    const int WIFI_RESET_FLAG_ADDR = 10;
};

#endif














































