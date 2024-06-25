#ifndef Command_h
#define Command_h

#include <arduino.h>
#include "FreeRTOS.h"
#include "globals.h"
#include "CommandParser.h"
#include "DataLogger.h"
#include "Driver_selectorValves.h"
#include "Driver_ProportionalValve.h"
#include "Driver_ppsystemsCO2.h"
#include "Driver_CO.h"
#include <RTClib.h>

#define CP_COMMANDS 16
#define CP_COMMAND_ARGS 4
#define CP_COMMAND_NAME_LENGTH 4
#define CP_COMMAND_ARG_SIZE 32
#define CP_RESPONSE_SIZE 64

//typedef CommandParser<CP_COMMANDS, CP_COMMAND_ARGS, CP_COMMAND_NAME_LENGTH, CP_COMMAND_ARG_SIZE, CP_RESPONSE_SIZE> FH_CommandParser;
typedef CommandParser<> FH_CommandParser;

extern Driver_selectorValves selectorValves;

extern Driver_ProportionalValve gasValve;
extern Driver_ProportionalValve oaValve;
extern Driver_ProportionalValve amValve;

extern Driver_ppsystemsCO2 co2;

extern Driver_CO co_1;
extern Driver_CO co_2;

extern DataLogger dataLogger;

extern RTC_PCF8523 rtc;

#define COMMAND_RESPONSE_MAX_SIZE 128 


class Command
{
    public:
        void init();
        void tick();
        void checkAndParseCommandLine(String commandLine);
        // command public methods
        bool setFlow(char ch, uint64_t flow);
        bool setSampleSet(uint set);
        bool enableLogFile(bool enable);
        bool setPIDk(char pid, char kConst, double value);
        bool setFlowManual(char pid, int64_t value);
        bool saveSettings();
        void restoreSettings();
        void co2Pump(bool enable);
        bool setPumpSpeed(int64_t value);
        void zeroCO2();
        void sendDataPacketFormat();
        bool setTime(String& datetime);
        void zeroCO();
        void spanCO(float spanPPM);
        bool flowZero(char flow);
        bool flowSpan(char flow, uint64_t intercept);

    private:
        FH_CommandParser parser;
        void respondOK();
        // command handler wrappers (overloads) for command parser
        static void setFlow(FH_CommandParser::Argument *args, char *response);
        static void setSampleSet(FH_CommandParser::Argument *args, char *response);
        static void enableLogFile(FH_CommandParser::Argument *args, char *response);
        static void setPIDk(FH_CommandParser::Argument *args, char *response);
        static void showPIDks(FH_CommandParser::Argument *args, char *response);
        static void setFlowManual(FH_CommandParser::Argument *args, char *response);
        static void saveSettings(FH_CommandParser::Argument *args, char *response);
        static void restoreSettings(FH_CommandParser::Argument *args, char *response);
        // TODO - test command to set zero flow
        // TODO - test command to set flow scaling factor
        static void setPumpSpeed(FH_CommandParser::Argument *args, char *response);
        static void zeroCO2(FH_CommandParser::Argument *args, char *response);
        static void sendDataPacketFormat(FH_CommandParser::Argument *args, char *response);
        static void setTime(FH_CommandParser::Argument *args, char *response);
        static void zeroCO(FH_CommandParser::Argument *args, char *response);
        static void spanCO(FH_CommandParser::Argument *args, char *response);
        static void flowZero(FH_CommandParser::Argument *args, char *response);
        static void flowSpan(FH_CommandParser::Argument *args, char *response);
        static void sendVersion(FH_CommandParser::Argument *args, char *response);
        static void send2co2(FH_CommandParser::Argument *args, char *response);
        // TODO - OPTIONAL Implement command for CO2 span/scale
        static void co2Pump(FH_CommandParser::Argument *args, char *response);
        RadioPacket packet;
        uint8_t commandBuffer[RADIO_COMMAND_QUEUE_RECORD_SIZE];
};

#endif
