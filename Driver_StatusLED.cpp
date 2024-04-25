#include "Driver_StatusLED.h"
#include "control.h"

void Driver_StatusLED::init(int16_t pin)
{
    pixel.setPin(pin);
    pixel.updateLength(1);
    pixel.begin();
    flashCount = 0;
    isOn = true;
    currStatus = CONTROL_STATE_IDLE;
}

void Driver_StatusLED::tick()
{
    setStatus();
    if (flashCount == 0)
    {
        if (flash == LED_FLASH_SLOW)
        {
            flashCount = LED_FLASH_SLOW_COUNT;
        }
        if (flash == LED_FLASH_FAST)
        {
            flashCount = LED_FLASH_FAST_COUNT;
        }
        isOn = !isOn;
    }
    if (flash == LED_FLASH_NONE)
    {
        pixel.fill(color1);
        pixel.show();
    }
    else
    {
        if (isOn)
        {
            pixel.fill(color1);          
            pixel.show();
        }
        else
        {
            pixel.fill(color2);
            pixel.show();
        }
        flashCount--;
    }
}


void Driver_StatusLED::setStatus()
{
    flash = LED_FLASH_FAST;

    if (!settings.baseStationAnswering)
    {
        // base station answering is green 
        color1 = pixel.Color(0, 255, 0);
    }
    else
    {
        // no radio contact is red 
        color1 = pixel.Color(255, 0, 0);
    }

    if (settings.co2State == 'M')
    {
        // CO2 measuring is blue 
        color2 = pixel.Color(0, 0, 255);
    }
    else
    {
        // Any other CO2 state is yellow 
        color2 = pixel.Color(255, 255, 0);
    }

}
