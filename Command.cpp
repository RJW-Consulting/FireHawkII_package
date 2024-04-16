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


bool  Command::setTime(String& datetime) {
    // Check if the string length is valid
    if (datetime.length() != 14 && datetime.length() != 16 && datetime.length() != 17) {
        return false;
    }

    // Check if all characters are digits
    for (char c : datetime) {
        if (!isdigit(c)) {
            return false;
        }
    }

    // Extract year, month, day, hour, minute, and second from the string
    int year, month, day, hour, minute, second;
    if (datetime.length() == 14) {
        year = datetime.substring(0, 4).toInt();
        month = datetime.substring(4, 6).toInt();
        day = datetime.substring(6, 8).toInt();
        hour = datetime.substring(8, 10).toInt();
        minute = datetime.substring(10, 12).toInt();
        second = datetime.substring(12).toInt();
    } else if (datetime.length() == 16) {
        // MM/DD/YYYY hh:mm:ss or YYYY/MM/DD hh:mm:ss format
        if (datetime.charAt(2) == '/' && datetime.charAt(5) == '/') {
            month = datetime.substring(0, 2).toInt();
            day = datetime.substring(3, 5).toInt();
            year = datetime.substring(6, 10).toInt();
            hour = datetime.substring(11, 13).toInt();
            minute = datetime.substring(14, 16).toInt();
            second = datetime.substring(17).toInt();
        } else if (datetime.charAt(4) == '/' && datetime.charAt(7) == '/') {
            year = datetime.substring(0, 4).toInt();
            month = datetime.substring(5, 7).toInt();
            day = datetime.substring(8, 10).toInt();
            hour = datetime.substring(11, 13).toInt();
            minute = datetime.substring(14, 16).toInt();
            second = datetime.substring(17).toInt();
        } else {
            return false; // Invalid format
        }
    } else if (datetime.length() == 17) {
        // YYYYMMDD hhmmss format
        year = datetime.substring(0, 4).toInt();
        month = datetime.substring(4, 6).toInt();
        day = datetime.substring(6, 8).toInt();
        hour = datetime.substring(9, 11).toInt();
        minute = datetime.substring(11, 13).toInt();
        second = datetime.substring(13, 15).toInt();
    }

    // Check if year, month, day, hour, minute, and second are within valid ranges
    if (year < 1900 || year > 9999 ||
        month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59) {
        return false;
    }

    // Check for valid days in a month
    if (day > 30 && (month == 4 || month == 6 || month == 9 || month == 11)) {
        return false;
    }
    if (day > 28 && month == 2 && (year % 4 != 0 || (year % 100 == 0 && year % 400 != 0))) {
        return false;
    }
    if (day > 29 && month == 2) {
        return false;
    }
    DateTime dt(year, month, day, hour, minute, second);
    rtc.stop();
    rtc.adjust(dt);
    rtc.start();


    return true;
}


void Command::setTime(FH_CommandParser::Argument *args, char *response)
{
    bool formatGood = false;

    String dateTimeString = args[0].asString;
    formatGood = command.setTime(dateTimeString);
    if (formatGood)
    {
        sprintf(responseBuff.stringPacket.chars,"Date and time set.");
    }
    else
    {
        sprintf(responseBuff.stringPacket.chars,"Invaled datetime.  Use YYYYMMDDhhmmss, MM/DD/YYYY hh:mm:ss, or YYYY/MM/DD hh:mm:ss.");
    }
}

void Command::zeroCO()
{
    co_1.zero();
    co_2.zero();
}

void Command::zeroCO(FH_CommandParser::Argument *args, char *response)
{
    command.zeroCO();
    sprintf(responseBuff.stringPacket.chars,"CO zero intercept set.");

}

void Command::spanCO(float spanPPM)
{
    co_1.span(spanPPM);
    co_2.span(spanPPM);
}

void Command::spanCO(FH_CommandParser::Argument *args, char *response)
{
    float span = (float) args[0].asDouble;
    command.spanCO(span);
    sprintf(responseBuff.stringPacket.chars,"CO slope set.");
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
    parser.registerCommand("co2z","", &zeroCO2);
    parser.registerCommand("coz","", &zeroCO);
    parser.registerCommand("cos","d", &spanCO);
    parser.registerCommand("df","", &sendDataPacketFormat);
    parser.registerCommand("st","s", &setTime);
}


