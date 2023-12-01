#include "Driver_mprls.h"

PressureSensor::PressureSensor(TCA9548A *mux, int mChannel, float *pVariable)
{
    i2cMux = mux;
    muxChannel = mChannel;
    pVar = pVariable;    
}

void PressureSensor::init()
{
    i2cMux->openChannel(muxChannel);

    if (! mpr.begin()) 
    {
        Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
        while (1) 
        {
            delay(10);
        }
    }

    i2cMux->closeChannel(muxChannel);
}


float PressureSensor::getPressure()
{
    float pValue = 0;
    i2cMux->openChannel(muxChannel);
    pValue =  mpr.readPressure();
    i2cMux->closeChannel(muxChannel);
    return pValue;
}

void PressureSensor::tick()
{
   *pVar = getPressure(); 
}
