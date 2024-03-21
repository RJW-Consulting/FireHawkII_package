#include "Driver_Pump.h"

Driver_Pump::Driver_Pump(Adafruit_MCP4728 *imcp, 
            uint_fast8_t ipumpChannel,
            bool *enabled,
            uint *pumpSpeed)
{
    enabledSetting = enabled;
    pumpSpeedSetting = pumpSpeed;
    mcp = imcp;
    pumpChannel = ipumpChannel;
    isEnabled = false;
}

void Driver_Pump::init()
{
    mcp->setChannelValue((MCP4728_channel_t) pumpChannel, (int) MIN_DAC);
}

void Driver_Pump::tick()
{
    if (*enabledSetting)
    {
        if (!isEnabled)
        {
            isEnabled = true;
            mcp->setChannelValue((MCP4728_channel_t) pumpChannel, (int) ((*pumpSpeedSetting * MAX_DAC) / 100));
            lastSpeed = *pumpSpeedSetting;
        }
        else
        {
            if (*pumpSpeedSetting != lastSpeed)
            {
                mcp->setChannelValue((MCP4728_channel_t) pumpChannel, (int) ((*pumpSpeedSetting * MAX_DAC) / 100));
                lastSpeed = *pumpSpeedSetting;
            }
        }
     }
     else
     {
        if (isEnabled)
        {
            isEnabled = false;
            mcp->setChannelValue((MCP4728_channel_t) pumpChannel, (int) MIN_DAC);
            lastSpeed = 0;
        }
     }
}
