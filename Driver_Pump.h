#ifndef Driver_Pump_h
#define Driver_Pump_h

#include "Arduino.h"
#include <Wire.h>
#include <stdint.h>
#include <Adafruit_MCP4728.h>
#include <TCA9548A-SOLDERED.h>
#include <PID_v1.h>
#include "globals.h"

class Driver_Pump
{
    public:
        Driver_Pump(Adafruit_MCP4728 *imcp, 
                    uint_fast8_t ipumpChannel,
                    bool *enabled,
                    uint *pumpSpeed);
        void init(); 
        void tick();
    private:
        Adafruit_MCP4728 *mcp;
        bool isEnabled = false;
        uint_fast8_t pumpChannel;
        uint lastSpeed;
        bool *enabledSetting;
        uint *pumpSpeedSetting;
};

#define MIN_DAC 0
#define MAX_DAC 4095




#endif