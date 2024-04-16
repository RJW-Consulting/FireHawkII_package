#include "Driver_CO.h"

void Driver_CO::init(DateTime *now, float *coReading_in, float *coVReading_in, float *slope_in, float *intercept_in, Adafruit_ADS1115 *theadc,  int adcChannel)
{
    adc = theadc;
    adc_Channel = adcChannel;
    coReading = coReading_in;
    vReading = coVReading_in;
    slope = slope_in;
    zeroV = intercept_in;
    coBuffer.initReadingBuffer(CO_BUFFER_SIZE);
    vBuffer.initReadingBuffer(CO_BUFFER_SIZE);
    nowPtr = now;
    lastReadingTime = *(this->nowPtr);
}

void Driver_CO::tick()
{
    getInstantReading();
    if (nowPtr->unixtime() > lastReadingTime.unixtime())
    {
        *coReading = coBuffer.getMeanReading();
        *vReading = vBuffer.getMeanReading();
        lastReadingTime = *nowPtr;
    }
}

void Driver_CO::zero()
{
    *zeroV = vBuffer.getMeanReading();
}

void Driver_CO::span(float spanPPM)
{
    *slope = (vBuffer.getMeanReading() - (*zeroV)) / spanPPM;
}

void Driver_CO::getInstantReading()
{
    int16_t coV;
    if (adc_Channel == 0)
    {
        coV = adc->readADC_Differential_0_1();
    }
    else
    {
        coV = adc->readADC_Differential_2_3();
    }
    float cov_f = (float)coV * ADC_VOLT_CONVERSION;
    vBuffer.addReading(cov_f);
    float co_f = (cov_f - (*zeroV))  / (*slope);
    coBuffer.addReading(co_f);
}


