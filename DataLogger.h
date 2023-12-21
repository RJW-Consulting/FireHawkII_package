#ifndef DataLogger_h
#define DataLogger_h

#include "Arduino.h" 
#include "string.h" 
#include <SPI.h>
#include <SD.h>
#include "minIni.h"
#include "RTClib.h"
#include "Driver_ppsystemsCO2.h"


#define SD_CARD_CS 10                                   

class DataLogger
{
    public:
        DataLogger();
        void init();
        void loadSettings();
        void setDateTime(DateTime *sysDateTime);
        void setCO2Driver(Driver_ppsystemsCO2 *sysCO2Driver);
        void setIntervalSecs(uint16_t intervalSecs);
        bool beginLogging();
        bool stopLogging();
        void tick();

    private:
        String logFileName;
        minIni *ini;
        File currentFile;
        DateTime *systemTime;
        DateTime lastLogTime;
        uint32_t logInterval;
        Driver_ppsystemsCO2 *co2Driver;

        String buildLogString();
        String getLogStringHeader();
        String logicalString(bool val);
        void fillDataPacket(struct DataPacket *packet);
        bool initSDCard();
};

#endif