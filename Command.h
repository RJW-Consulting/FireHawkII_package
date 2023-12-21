#ifndef Command_h
#define Command_h

#include <arduino.h>
#include "FreeRTOS_SAMD21.h"
#include "globals.h"
#include "CommandParser.h"
#include "DataLogger.h"
#include "Driver_selectorValves.h"


#define CP_COMMANDS 16
#define CP_COMMAND_ARGS 4
#define CP_COMMAND_NAME_LENGTH 4
#define CP_COMMAND_ARG_SIZE 32
#define CP_RESPONSE_SIZE 64

//typedef CommandParser<CP_COMMANDS, CP_COMMAND_ARGS, CP_COMMAND_NAME_LENGTH, CP_COMMAND_ARG_SIZE, CP_RESPONSE_SIZE> FH_CommandParser;
typedef CommandParser<> FH_CommandParser;

extern Driver_selectorValves selectorValves;

extern DataLogger dataLogger;

class Command
{
    public:
        void init();
        void tick();
        void checkAndParseCommandLine(String commandLine);

    private:
        FH_CommandParser parser;
        char responseBuff[FH_CommandParser::MAX_RESPONSE_SIZE];
        // command handlers
        static void setFlow(FH_CommandParser::Argument *args, char *response);
        static void setSampleSet(FH_CommandParser::Argument *args, char *response);
        static void enableLogFile(FH_CommandParser::Argument *args, char *response);
};

#endif
