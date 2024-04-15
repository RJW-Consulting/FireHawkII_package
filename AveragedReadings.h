#ifndef Averagedreadings_h
#define Averagedreadings_h

#include "Arduino.h"
#include "FreeRTOS.h"
#include <stdint.h>
#include <vector>
#include <iostream>
#include "globals.h"

class AveragedReadings
{
    public:
        void initReadingBuffer(uint_fast8_t buffersize);
        void addReading(float reading);
        float getMeanReading();
    private:
        float *buffer;
        uint_fast8_t maxReadings;
        uint_fast8_t bufferSize;
        uint_fast8_t bufferHead;
        uint_fast8_t bufferTail;
        float bufferSum;
};

#endif