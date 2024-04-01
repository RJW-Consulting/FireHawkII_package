#ifndef DataLogger_h
#define DataLogger_h

#include "Arduino.h" 
#include "string.h" 
#include <SPI.h>
#include <SD.h>
#include "minIni.h"
#include "RTClib.h"
#include "FreeRTOS_SAMD21.h"
#include "Driver_ppsystemsCO2.h"
#include "Driver_ProportionalValve.h"


#define SD_CARD_CS 10                                   

class DataLogger
{
    public:
        DataLogger();
        void init();
        void loadSettings();
        bool saveSettings();
        void setDateTime(DateTime *sysDateTime);
        void setCO2Driver(Driver_ppsystemsCO2 *sysCO2Driver);
        void setIntervalSecs(uint16_t intervalSecs);
        bool beginLogging();
        bool stopLogging();
        uint16_t dataPacketSize();
        String getLogDataTypes();
        String getLogStringHeader();
        void tick();

    private:
        String logFileName;
        File currentFile;
        DateTime *systemTime;
        DateTime lastLogTime;
        uint32_t logInterval;
        Driver_ppsystemsCO2 *co2Driver;

        String buildLogString();
        String logicalString(bool val);
        void fillDataPacket(struct DataPacket *packet);
        bool initSDCard();
        DataPacket packet;

};

extern Driver_ProportionalValve gasValve;
extern Driver_ProportionalValve oaValve;
extern Driver_ProportionalValve amValve;

extern QueueHandle_t handle_command_queue;
extern QueueHandle_t handle_command_response_queue;
extern QueueHandle_t handle_data_queue;

#endif