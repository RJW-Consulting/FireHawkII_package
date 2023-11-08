
#ifndef Driver_StatusLED_h
#define Driver_StatusLED_h

#include <stdint.h>
#include <stdbool.h>
#include <Adafruit_NeoPixel.h>

#define LED_FLASH_NONE 0
#define LED_FLASH_SLOW 1
#define LED_FLASH_FAST 2

#define LED_FLASH_SLOW_COUNT 100
#define LED_FLASH_FAST_COUNT 10

class Driver_StatusLED
{
    public:
        void init(int16_t pin);
        void tick();

        void setStatus(uint8_t status);

    private:
        Adafruit_NeoPixel pixel;
        uint8_t flash;
        uint32_t color;
        uint16_t flashCount;
        bool isOn;
        uint8_t currStatus;
};

#endif