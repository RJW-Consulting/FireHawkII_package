#include "Command.h"

        
        

void Command::tick()
{

}

void Command::checkAndParseCommandLine(String commandLine)
{
    parser.processCommand(commandLine.c_str(), responseBuff);
    Serial.println(responseBuff);
}

void Command::setFlow(FH_CommandParser::Argument *args, char *response)
{
    String channel = args[0].asString;
    uint64_t flow = args[1].asUInt64;
    char ch = channel.charAt(0);

    switch (ch)
    {
        case 'g':
            settings.flowGasSetPoint = flow;
            break;
        case 'o':
            settings.flowOaSetPoint = flow;
            break;
        case 'a':
            settings.flowAmSetPoint = flow;
            break;
        default:
            strcpy(response, "No such channel as ");
            strcat(response, channel.c_str());
            break;
    }

}

void Command::setSampleSet(FH_CommandParser::Argument *args, char *response)
{
    uint64_t set = args[0].asUInt64;
    char rbuff[64];
 
    if (set > 3)
        strcpy(response, "set must be 0-3");
    else{
        if (set == 0)
        {
            selectorValves.closeGang(0);
            selectorValves.closeGang(1);
            selectorValves.closeGang(2);
            strcpy(response, "All selector valves closed");
        }
        else
        {
            selectorValves.openSet(set-1);
            sprintf(rbuff,"Opened sample set %u", set);
            strcpy(response, rbuff); 
        }
        readings.sampleSet = set;
    }
   
}

void Command::enableLogFile(FH_CommandParser::Argument *args, char *response)
{
    uint64_t enable = args[0].asUInt64;
    if (enable)
    {
        dataLogger.beginLogging();
        strcpy(response, "Logging on");
    }
    else
    {
        dataLogger.stopLogging();
        strcpy(response, "Logging off");
    }

}

void Command::init()
{
    if (!parser.registerCommand("sf","su", &setFlow))
        printf("Could not register set flow command");
    if (!parser.registerCommand("ss","u", &setSampleSet))
        printf("Could not register set sample command");
   if (!parser.registerCommand("le","u", &enableLogFile))
        printf("Could not register enable log file command");

}

