#include "Command.h"

// command response buffer needs to be global due to static nature of 
// command parser command handlers        
union RadioPacket responseBuff;
char responseType;       

extern Command command;
extern DataLogger dataLogger;
void Command::tick()
{
    if (xQueueReceive(handle_command_queue, (void *) &commandBuffer, 0))
    {
        String command((char *) commandBuffer);
        // command handlers will change the response type if needed
        responseType = 'R';
        checkAndParseCommandLine(command);
        responseBuff.stringPacket.packetType = responseType;
        xQueueSend(handle_command_response_queue, &responseBuff, portMAX_DELAY);        
    }
}

void Command::checkAndParseCommandLine(String commandLine)
{
    parser.processCommand(commandLine.c_str(), responseBuff.stringPacket.chars);
    Serial.println(responseBuff.stringPacket.chars);
}

void Command::respondOK()
{
    strcpy(responseBuff.stringPacket.chars, "OK");
}

void Command::setFlow(FH_CommandParser::Argument *args, char *response)
{
    String channel = args[0].asString;
    uint64_t flow = args[1].asUInt64;
    char ch = channel.charAt(0);
    command.setFlow(ch, flow);
    strcpy(response, responseBuff.stringPacket.chars);
}


bool Command::setFlow(char ch, uint64_t flow)
{
    bool retval = true;
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
            sprintf(responseBuff.stringPacket.chars, "No such channel as %c", ch);
            retval = false;
            break;
    }
    if (retval)
    {
        respondOK();
    }
    return retval;
}

void Command::setSampleSet(FH_CommandParser::Argument *args, char *response)
{
    uint64_t set = args[0].asUInt64;
    command.setSampleSet(set);
    strcpy(response, responseBuff.stringPacket.chars);
}

bool Command::setSampleSet(uint set)
{
    bool retval = false;
    if (set > 4)
        strcpy(responseBuff.stringPacket.chars, "set must be 0-4");
    else
    {
        if (set == 0)
        {
            settings.samplePumpOn = false;
            selectorValves.closeGang(0);
            selectorValves.closeGang(1);
            selectorValves.closeGang(2);
            strcpy(responseBuff.stringPacket.chars, "All selector valves closed");
            retval = true;
        }
        else if (set == 4)
        {
            settings.samplePumpOn = true;
            selectorValves.closeGang(0);
            selectorValves.closeGang(1);
            selectorValves.closeGang(2);
            strcpy(responseBuff.stringPacket.chars, "All selector valves closed, pump on");
            retval = true;
        }
        else
        {
            selectorValves.openSet(set-1);
            settings.samplePumpOn = true;
            sprintf(responseBuff.stringPacket.chars,"Opened sample set %u", set);
            retval = true;
        }
        readings.sampleSet = set;
    }
    return retval;   
}

void Command::enableLogFile(FH_CommandParser::Argument *args, char *response)
{
    uint64_t enable = args[0].asUInt64;
    command.enableLogFile((bool) enable);
    strcpy(response, responseBuff.stringPacket.chars);
}

bool Command::enableLogFile(bool enable)
{
    if (enable)
    {
        dataLogger.beginLogging();
        strcpy(responseBuff.stringPacket.chars, "Logging on");
    }
    else
    {
        dataLogger.stopLogging();
        strcpy(responseBuff.stringPacket.chars, "Logging off");
    }
    return true;
}

void Command::setPIDk(FH_CommandParser::Argument *args, char *response)
{
    response[0] = 0;
    String pidS = args[0].asString;
    String constS = args[1].asString;
    double value = args[2].asDouble;
    command.setPIDk(pidS.charAt(0), constS.charAt(0), value);
    return;
    strcpy(response, responseBuff.stringPacket.chars);
}

bool Command::setPIDk(char pid, char kConst, double value)
{
    bool retval = true;
    bool badP = false;
    bool badK = false;

    switch (pid)
    {
        case 'g':
            switch (kConst)
            {
                case 'p':
                    settings.gasPIDKp = value;
                    break;
                case 'i':
                    settings.gasPIDKi = value;
                    break;
                case 'd':
                    settings.gasPIDKd = value;
                    break;
                default:
                    badK = true;
                    break;
            }
            if (!badK)
                gasValve.updateKs();
            break;
        case 'o':
            switch (kConst)
            {
                case 'p':
                    settings.oaPIDKp = value;
                    break;
                case 'i':
                    settings.oaPIDKi = value;
                    break;
                case 'd':
                    settings.oaPIDKd = value;
                    break;
                default:
                    badK = true;
                    break;
            }
            if (!badK)
                oaValve.updateKs();
            break;
        case 'a':
            switch (kConst)
            {
                case 'p':
                    settings.amPIDKp = value;
                    break;
                case 'i':
                    settings.amPIDKi = value;
                    break;
                case 'd':
                    settings.amPIDKd = value;
                    break;
                default:
                    badK = true;
                    break;
            }
            if (!badK)
                amValve.updateKs();
            break;
        default:
            badP = true;
    }
    responseBuff.stringPacket.chars[0] = 0;
    if (badP)
    {
        sprintf(responseBuff.stringPacket.chars,"PID should be g,o, or a, not %c", pid);
        retval = false;
    }
    else if (badK)
    {
        sprintf(responseBuff.stringPacket.chars,"K should be p,i, or d, not %c", kConst);
        retval = false;
    }
    else
    {
        //sprintf(responseBuff,"PID %c K%c set to %9.6lf .", pid, kConst, value);
        //sprintf(responseBuff,"PID %c K%c set", pid, kConst);
    }
    return retval;
}

void Command::saveSettings(FH_CommandParser::Argument *args, char *response)
{
    uint64_t enable = args[0].asUInt64;
    command.saveSettings();
    strcpy(response, responseBuff.stringPacket.chars);
}

bool Command::saveSettings()
{
    bool retval = dataLogger.saveSettings();

    if (retval)
        strcpy(responseBuff.stringPacket.chars,"Settings saved");
    else
        strcpy(responseBuff.stringPacket.chars,"Save to settings file FAILED");
    
    return retval;

}

void Command::setPumpSpeed(FH_CommandParser::Argument *args, char *response)
{
    int64_t value = args[0].asInt64;
    if (value > 100 || value < 0)
        strcpy(response, "Pump speed must be 0-100");
    else
    {
        command.setPumpSpeed(value);
        strcpy(response, responseBuff.stringPacket.chars);
    }
}

bool Command::setPumpSpeed(int64_t value)
{
    settings.samplePumpSpeed = (uint) value;
    sprintf(responseBuff.stringPacket.chars,"pump speed set to %d %%", (int) value);
    return true;
}

void Command::setFlowManual(FH_CommandParser::Argument *args, char *response)
{
    String pidS = args[0].asString;
    int64_t value = args[1].asInt64;
    if (value > 4096)
        value = -1;
    command.setFlowManual(pidS.charAt(0), value);
    strcpy(response, responseBuff.stringPacket.chars);
}

bool Command::setFlowManual(char pid, int64_t value)
{
    bool retval = true;
    bool badP = false;

    switch (pid)
    {
        case 'g':
            gasValve.setManual(value);
            break;
        case 'o':
            oaValve.setManual(value);
            break;
        case 'a':
            amValve.setManual(value);
            break;
        default:
            badP = true;
    }
    responseBuff.stringPacket.chars[0] = 0;
    if (badP)
    {
        sprintf(responseBuff.stringPacket.chars,"valve should be g,o, or a, not %c", pid);
        retval = false;
    }
    else
    {
        sprintf(responseBuff.stringPacket.chars,"Valve %c manual set to %I64d.", pid, value);
        //sprintf(responseBuff,"PID %c K%c set", pid, kConst);
    }
    return retval;
}

void Command::zeroCO2()
{
    co2.startZero();
}

void Command::zeroCO2(FH_CommandParser::Argument *args, char *response)
{
    command.zeroCO2();
}

void Command::sendDataPacketFormat(FH_CommandParser::Argument *args, char *response)
{
    command.sendDataPacketFormat();
}

void Command::sendDataPacketFormat()
{
    String format = dataLogger.getLogDataTypes();
    strcpy(responseBuff.stringPacket.chars, format.c_str()); 
    responseType= 'F';
}

void Command::init()
{
    parser.registerCommand("sf","su", &setFlow);
    parser.registerCommand("sm","si", &setFlowManual);
    parser.registerCommand("ss","u", &setSampleSet);
    parser.registerCommand("le","u", &enableLogFile);
    parser.registerCommand("pk","ssd", &setPIDk);
    parser.registerCommand("set","", &saveSettings);
    parser.registerCommand("ps","u", &setPumpSpeed);
    parser.registerCommand("zco2","", &zeroCO2);
    parser.registerCommand("df","", &sendDataPacketFormat);
}


