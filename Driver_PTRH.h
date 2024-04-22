#ifndef Driver_PTRH_h
#define Driver_PTRH_h

#include "Arduino.h" 
#include <Adafruit_MS8607.h>
#include "RTClib.h"
#include "globals.h"

// TODO - Replace PTRH sensor and check RH reading

class Driver_PTRH
{
    public:
        void init(DateTime *nowPtr, Adafruit_MS8607 *sensorPtr);
        void tick();

    private:
        bool initialized;
        DateTime *now;
        Adafruit_MS8607 *sensor;
        uint32_t lastReadingTime;
        sensors_event_t temp;
        sensors_event_t pressure;
        sensors_event_t humidity;
};

#endif