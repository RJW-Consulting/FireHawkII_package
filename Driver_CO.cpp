#include "Driver_CO.h"

void Driver_CO::init(DateTime *now, float *coReading_in, float *coVReading_in, float *op1Reading_in, float *op2Reading_in, float *slope_in, float *intercept_in, Adafruit_ADS1115 *theadc,  int adcChannel)
{
    adc = theadc;
    adc_Channel = adcChannel;
    coReading = coReading_in;
    vReading = coVReading_in;
    vOP1Reading = op1Reading_in;
    vOP2Reading= op2Reading_in;
    slope = slope_in;
    zeroV = intercept_in;
    coBuffer.initReadingBuffer(CO_BUFFER_SIZE);
    vBuffer.initReadingBuffer(CO_BUFFER_SIZE);
    vOP1Buffer.initReadingBuffer(CO_BUFFER_SIZE);
    vOP2Buffer.initReadingBuffer(CO_BUFFER_SIZE);
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
        *vOP1Reading = vOP1Buffer.getMeanReading();
        *vOP2Reading = vOP2Buffer.getMeanReading();
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
    int16_t coV_OP1;
    int16_t coV_OP2;
    if (adc_Channel == 0)
    {
        coV = adc->readADC_Differential_0_1();
        coV_OP1 = adc->readADC_SingleEnded(0);
        coV_OP2 = adc->readADC_SingleEnded(1);
    }
    else
    {
        coV = adc->readADC_Differential_2_3();
    }
    float cov_f = (float)coV * ADC_VOLT_CONVERSION;
    vBuffer.addReading(cov_f);
    float co_f = (cov_f - (*zeroV))  / (*slope);
    coBuffer.addReading(co_f);
    float covOP1 =  (float)coV_OP1 * ADC_VOLT_CONVERSION;
    vOP1Buffer.addReading(covOP1);
    float covOP2 =  (float)coV_OP2 * ADC_VOLT_CONVERSION;
    vOP2Buffer.addReading(covOP2);
    
}


