#include "DataLogger.h"

DataLogger::DataLogger()
{
    logFileName = "";
    if (!SD.begin(SD_CARD_CS)) 
        Serial.println("Card failed, or not present");
    else
        Serial.println("card initialized.");
    logInterval = 1;
    //currentFile = (SDLib::File) NULL;
}

void DataLogger::setDateTime(DateTime *sysDateTime)
{
    systemTime = sysDateTime;
    lastLogTime = *systemTime;

}


void DataLogger::setCO2Driver(Driver_ppsystemsCO2 *sysCO2Driver)
{
    co2Driver = sysCO2Driver;
}


void DataLogger::setIntervalSecs(uint16_t intervalSecs)
{
    logInterval = intervalSecs;
}


bool DataLogger::beginLogging()
{
    char buffer[8];
    bool retVal = false;

    sprintf(buffer, "d%02d%02d%02d",
     systemTime->year() % 100,
     systemTime->month(),
     systemTime->day());

    logFileName = "d";
    logFileName += buffer;
    logFileName += ".dat";

    currentFile = SD.open(logFileName, FILE_WRITE);
    if (!currentFile)
    {
        Serial.printf("Cannot open file %s", logFileName);
    }
    else 
    {
        retVal = true;
    }
    return retVal;
}


bool DataLogger::stopLogging()
{
    if (currentFile)
    {
        currentFile.close();
        //currentFile = NULL;
    }
    return true;
}


void DataLogger::tick()
{
    if ((systemTime->unixtime() - lastLogTime.unixtime()) >= logInterval)
    {
        String logLine = buildLogString();
        if (currentFile)
        {
            currentFile.write(logLine.c_str());
            lastLogTime = *systemTime;
        }
    } 
}


String DataLogger::buildLogString()
{
    char buffer[32];
    String line;

    line += systemTime->timestamp();
    line += ",";
    line += (co2Driver->getState() + ",");
    if (co2Driver->getState() == 'M')
    {
        DateTime mTime;
        float measurement = co2Driver->getMeasurement(mTime);
        sprintf(buffer, "%5.1f", measurement);
        line += buffer;
        line += ",";
    }
    else
    {
        line += "***,";
    }
    line += "\n";

    return line;
}


