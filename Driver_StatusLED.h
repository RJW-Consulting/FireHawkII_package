
#ifndef Driver_StatusLED_h
#define Driver_StatusLED_h

#include <stdint.h>
#include <stdbool.h>
#include <Adafruit_NeoPixel.h>
#include "globals.h"

#define LED_FLASH_NONE 0
#define LED_FLASH_SLOW 1
#define LED_FLASH_FAST 2

#define LED_FLASH_SLOW_COUNT 100
#define LED_FLASH_FAST_COUNT 5

class Driver_StatusLED
{
    // TODO - Flesh out status LED
    public:
        void init(int16_t pin);
        void tick();

        void setStatus();

    private:
        Adafruit_NeoPixel pixel;
        uint8_t flash;
        uint32_t color1;
        uint32_t color2;
        uint16_t flashCount;
        bool isOn;
        uint8_t currStatus;
};

#endif