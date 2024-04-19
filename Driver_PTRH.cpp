#include "Driver_PTRH.h"

void Driver_PTRH::init(DateTime *nowPtr, Adafruit_MS8607 *sensorPtr)
{
    now = nowPtr;
    lastReadingTime = nowPtr->unixtime();
    sensor = sensorPtr;
    if (!sensor->begin()) 
    {
        Serial.println("Failed to find MS8607 chip");
        initialized = false;
    }
    else
    {
        initialized = true;
        Serial.println("MS8607 chip initialized");
    }
    sensor->setHumidityResolution(MS8607_HUMIDITY_RESOLUTION_OSR_8b);
    sensor->setPressureResolution(MS8607_PRESSURE_RESOLUTION_OSR_4096);
}

void Driver_PTRH::tick()
{
    if (now->unixtime() > lastReadingTime)
    {
        sensor->getEvent(&pressure, &temp, &humidity);
        readings.airTemp = temp.temperature;
        readings.pressure = pressure.pressure;
        readings.rh = humidity.relative_humidity;
        lastReadingTime = now->unixtime();
    }

}
