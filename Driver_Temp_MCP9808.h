#ifndef Driver_TEMP_MCP9808_h
#define Driver_TEMP_MCP9808_h

#include "Arduino.h" 
#include <Adafruit_MCP9808.h>
#include "RTClib.h"
#include "globals.h"


class Driver_Temp_MCP9808
{
    public:
        void init(DateTime *nowPtr, Adafruit_MCP9808 *sensorPtr, uint8_t sensor_I2C);
        void tick();

    private:
        bool initialized;
        DateTime *now;
        Adafruit_MCP9808 *sensor;
        uint32_t lastReadingTime;
        double temp;
};

#endif