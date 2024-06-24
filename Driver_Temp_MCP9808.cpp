#include "Driver_Temp_MCP9808.h"

void Driver_Temp_MCP9808::init(DateTime *nowPtr, Adafruit_MCP9808 *sensorPtr, uint8_t sensor_I2C)
{
    now = nowPtr;
    lastReadingTime = nowPtr->unixtime();
    sensor = sensorPtr;
    if (!sensor->begin(sensor_I2C)) 
    {
        Serial.println("Failed to find MCP9808 chip");
        initialized = false;
    }
    else
    {
        initialized = true;
        Serial.println("MCP9808 chip initialized");
        sensor->setResolution(1);
    }
}

void Driver_Temp_MCP9808::tick()
{
    if (now->unixtime() > lastReadingTime)
    {
        temp = sensor->readTempC();
        readings.caseTemp = temp;
    }

}
