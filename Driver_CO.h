#ifndef Driver_CO_h
#define Driver_CO_h

#include "Arduino.h"
#include "FreeRTOS.h"
#include <stdint.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MS8607.h>
#include "AveragedReadings.h"
#include "globals.h"

class Driver_CO
{
    public:
        void init(DateTime *now, float *coReading_in, float *coVReading_in, float *slope_in, float *intercept_in, Adafruit_ADS1115 *theadc,  int adcChannel);
        void tick();
        void zero();
        void span(float spanPPM);
    
    private:
        DateTime *nowPtr;
        DateTime lastReadingTime;
        DateTime lastZeroTime;
        void getInstantReading();
        Adafruit_ADS1115 *adc;
        int adc_Channel;
        AveragedReadings coBuffer;
        AveragedReadings vBuffer;
        float *zeroV;
        float *slope;
        float *coReading;
        float *vReading;
};

#define CO_BUFFER_SIZE 4

#define ADC_VOLT_CONVERSION 0.000188058298072402

#endif
