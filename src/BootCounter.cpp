#include "BootCounter.h"

BootCounter::BootCounter() {}

void BootCounter::begin() {
    EEPROM.begin(EEPROM_SIZE);

    int count = EEPROM.read(BOOT_COUNT_ADDR);
    
    if (count < 0 || count > 10) {
        count = 0;
    }

    count++;
    EEPROM.write(BOOT_COUNT_ADDR, count);
    EEPROM.commit();
}

int BootCounter::getCount() {
    return EEPROM.read(BOOT_COUNT_ADDR);
}

bool BootCounter::shouldResetWifi() {
    return getCount() >= 5;
}

void BootCounter::resetBootCounter() {
    EEPROM.write(BOOT_COUNT_ADDR, 0);
    EEPROM.commit();
}

void BootCounter::resetEverything() {
    EEPROM.write(BOOT_COUNT_ADDR, 0);
    EEPROM.write(WIFI_RESET_FLAG_ADDR, 1); 
    EEPROM.commit();
}
