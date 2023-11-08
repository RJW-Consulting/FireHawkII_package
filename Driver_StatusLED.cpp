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
        pixel.fill(color);
        pixel.show();
    }
    else
    {
        if (isOn)
        {
            pixel.fill(color);          
            pixel.show();
        }
        else
        {
            pixel.fill(pixel.Color(0,0,0));
            pixel.show();
        }
        flashCount--;
    }
}

/*
#define CONTROL_STATE_IDLE
#define CONTROL_STATE_WAIT_TRIGGER
#define CONTROL_STATE_SAMPLE_0
#define CONTROL_STATE_SAMPLE_1
#define CONTROL_STATE_SAMPLE_2
#define CONTROL_STATE_SAMPLE_CO2_ZERO
#define CONTROL_STATE_SAMPLE_CO2_SPAN
#define CONTROL_STATE_SAMPLE_CO_ZERO
#define CONTROL_STATE_SAMPLE_CO_SPAN
#define CONTROL_STATE_SENDING_LOG
*/

void Driver_StatusLED::setStatus(uint8_t status)
{
    if (status != currStatus)
    {
        switch (status)
        {
            case CONTROL_STATE_IDLE:
                color = pixel.Color(0, 255, 0);
                flash = LED_FLASH_NONE;
                break;
            case CONTROL_STATE_WAIT_TRIGGER:
                color = pixel.Color(0, 255, 0);
                flash = LED_FLASH_SLOW;
                break;
            case CONTROL_STATE_SAMPLE_0:
                color = pixel.Color(255, 0, 0);
                flash = LED_FLASH_FAST;
                break;
            case CONTROL_STATE_SAMPLE_1:
                color = pixel.Color(255, 255, 0);
                flash = LED_FLASH_FAST;
                break;
            case CONTROL_STATE_SAMPLE_2:
                color = pixel.Color(0, 255, 255);
                flash = LED_FLASH_FAST;
                break;
            case CONTROL_STATE_SAMPLE_CO2_ZERO:
                color = pixel.Color(0, 0, 255);
                flash = LED_FLASH_FAST;
                break;
            case CONTROL_STATE_SAMPLE_CO2_SPAN:
                color = pixel.Color(0, 0, 255);
                flash = LED_FLASH_SLOW;
                break;
            case CONTROL_STATE_SAMPLE_CO_ZERO:
                color = pixel.Color(255, 0, 255);
                flash = LED_FLASH_FAST;
                break;
            case CONTROL_STATE_SAMPLE_CO_SPAN:
                color = pixel.Color(255, 0, 255);
                flash = LED_FLASH_SLOW;
                break;
            case CONTROL_STATE_SENDING_LOG:
                color = pixel.Color(255, 255, 255);
                flash = LED_FLASH_FAST;
                break;
        }
        currStatus = status;
    }
    flashCount = 0;
}
