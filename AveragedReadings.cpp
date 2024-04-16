#include "AveragedReadings.h"

void AveragedReadings::initReadingBuffer(uint_fast8_t buffersize)
{
    buffer = (float *) pvPortMalloc(buffersize * sizeof(float));

    bufferHead = 0;
    bufferTail = 0;
    bufferSize = 0;
    bufferSum = 0;
    if (buffer)
    {
        maxReadings = buffersize;
        for (uint i = 0; i < bufferSize; i++)
        {
            buffer[i] = 0;
        } 
    } 
}

void AveragedReadings::addReading(float reading)
{
    if (bufferSize == maxReadings) 
    {
        // Remove the oldest data point if the buffer is full
        bufferSum -= buffer[bufferHead];
        bufferHead = (bufferHead + 1) % maxReadings;
    }

    buffer[bufferTail] = reading;
    bufferTail = (bufferTail + 1) % maxReadings;
    bufferSize++;
    if (bufferSize > maxReadings)
      bufferSize = maxReadings;
    bufferSum += reading;
}

float AveragedReadings::getMeanReading()
{
    if (bufferSize == 0) {
      return 0; // Avoid division by zero when the buffer is empty
    }
    return (float)(bufferSum / bufferSize);

}

