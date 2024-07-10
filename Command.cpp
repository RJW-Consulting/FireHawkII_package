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
        case 's':
            settings.flowGasSetPoint = flow;
            break;
        case 'q':
            settings.flowOaSetPoint = flow;
            break;
        case 't':
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
    if (readings.sampleSet != set)
        co2.resetAccumulators();
    if ((readings.batteryV < settings.battThreshold) && (set != 0))
    {
        String rstring = "Battery below "+String(settings.battThreshold)+"V threshold.  Cannnot open sample set.";
        strcpy(responseBuff.stringPacket.chars, rstring.c_str());
    }
    else
    {
        if (set > 4)
        {
            strcpy(responseBuff.stringPacket.chars, "set must be 0-4");
        }
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

void Command::showPIDks(FH_CommandParser::Argument *args, char *response)
{
    response[0] = 0;
    sprintf(responseBuff.stringPacket.chars,"PIDs: Sorb P=%5.3lf, I=%5.3lf, D=%5.3lf  Tef P=%5.3lf, I=%5.3lf, D=%5.3lf  Quar P=%5.3lf, I=%5.3lf, D=%5.3lf", settings.gasPIDKp, settings.gasPIDKi, settings.gasPIDKd,settings.amPIDKp, settings.amPIDKi, settings.amPIDKd, settings.oaPIDKp, settings.oaPIDKi, settings.oaPIDKd );
    strcpy(response, responseBuff.stringPacket.chars);
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
        case 's':
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
        case 'q':
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
        case 't':
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
        sprintf(responseBuff.stringPacket.chars,"PID should be s,q, or t, not %c", pid);
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

void Command::restoreSettings(FH_CommandParser::Argument *args, char *response)
{
    uint64_t enable = args[0].asUInt64;
    command.restoreSettings();
    strcpy(response, responseBuff.stringPacket.chars);
}

void Command::restoreSettings()
{
    dataLogger.loadSettings();

    strcpy(responseBuff.stringPacket.chars,"Settings restored from settings file.");
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
        case 's':
            gasValve.setManual(value);
            break;
        case 'q':
            oaValve.setManual(value);
            break;
        case 't':
            amValve.setManual(value);
            break;
        default:
            badP = true;
    }
    responseBuff.stringPacket.chars[0] = 0;
    if (badP)
    {
        sprintf(responseBuff.stringPacket.chars,"valve should be s,q, or t, not %c", pid);
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
    //co_2.zero();
}

void Command::zeroCO(FH_CommandParser::Argument *args, char *response)
{
    command.zeroCO();
    sprintf(responseBuff.stringPacket.chars,"CO zero intercept set.");
}

void Command::spanCO(float spanPPM)
{
    co_1.span(spanPPM);
    //co_2.span(spanPPM);
}

void Command::spanCO(FH_CommandParser::Argument *args, char *response)
{
    float span = (float) args[0].asDouble;
    command.spanCO(span);
    sprintf(responseBuff.stringPacket.chars,"CO slope set.");
}

void Command::co2Pump(bool enable)
{
    co2.setPump(enable);
}

void Command::co2Pump(FH_CommandParser::Argument *args, char *response)
{
    int64_t value = args[0].asInt64;
    bool enable = false;
    if (value != 0)
        enable = true;
    command.co2Pump(enable);
    if (enable)
        sprintf(responseBuff.stringPacket.chars,"CO2 pump ON.");
    else
        sprintf(responseBuff.stringPacket.chars,"CO2 pump OFF.");

}

bool Command::flowZero(char flow)
{
    bool retval = true;
    bool badf = false;

    switch (flow)
    {
        case 's':
            settings.gasFlowIntercept = -(readings.flowGas);
            break;
        case 'q':
            settings.oaFlowIntercept = -(readings.flowOa);
            break;
        case 't':
            settings.amFlowIntercept = -(readings.flowAm);
            break;
        default:
            badf = true;
    }
    responseBuff.stringPacket.chars[0] = 0;
    if (badf)
    {
        sprintf(responseBuff.stringPacket.chars,"Flow should be s,q, or t, not %c", flow);
        retval = false;
    }
    else
    {
        sprintf(responseBuff.stringPacket.chars,"Flow %c zero set.", flow);
        //sprintf(responseBuff,"PID %c K%c set", pid, kConst);
    }
    return retval;
}

void Command::flowZero(FH_CommandParser::Argument *args, char *response)
{
    String pidS = args[0].asString;
    command.flowZero(pidS.charAt(0));
    strcpy(response, responseBuff.stringPacket.chars);
}

bool Command::flowSpan(char flow, uint64_t correctFlow)
{
    bool retval = true;
    bool badf = false;

    switch (flow)
    {
        case 's':
            if (correctFlow > 0)
                settings.gasFlowSlope = ((float)correctFlow / (float)readings.flowGas);
            else
                settings.gasFlowSlope = 1;
            break;
        case 'q':
            if (correctFlow > 0)
                settings.oaFlowSlope = ((float)correctFlow / (float)readings.flowOa);
            else
                settings.oaFlowSlope = 1; 
            break;
        case 't':
            if (correctFlow > 0)
                settings.amFlowSlope = ((float)correctFlow / (float)readings.flowAm);
            else
                settings.amFlowSlope = 1;
            break;
        default:
            badf = true;
    }
    responseBuff.stringPacket.chars[0] = 0;
    if (badf)
    {
        sprintf(responseBuff.stringPacket.chars,"Flow should be s,q, or t, not %c", flow);
        retval = false;
    }
    else
    {
        sprintf(responseBuff.stringPacket.chars,"Flow %c slope set.", flow);
        //sprintf(responseBuff,"PID %c K%c set", pid, kConst);
    }
    return retval;
}

void Command::flowSpan(FH_CommandParser::Argument *args, char *response)
{
    String channel = args[0].asString;
    uint64_t flow = args[1].asUInt64;
    char ch = channel.charAt(0);
    command.flowSpan(ch, flow);
    strcpy(response, responseBuff.stringPacket.chars);
}

void Command::sendVersion(FH_CommandParser::Argument *args, char *response)
{
    response[0] = 0;
    strcpy(response, versionString.c_str());
}

void Command::send2co2(FH_CommandParser::Argument *args, char *response)
{
    response[0] = 0;
    String command = args[0].asString;
    co2.send(command);
    String rstring = "Sent command \""+command+"\" to CO2 monitor.";
    strcpy(response, rstring.c_str());
}

void Command::co2background(FH_CommandParser::Argument *args, char *response)
{
    response[0] = 0;
    float background = (float) args[0].asDouble;
    if (background < 0)
    {
        background = readings.co2Conc;
    }
    settings.co2background = background;

    String rstring = "CO2 background set to "+String(background)+" PPM";

    strcpy(response, rstring.c_str());
}

void Command::setBattThreshold(FH_CommandParser::Argument *args, char *response)
{
    response[0] = 0;
    float threshold = (float) args[0].asDouble;    
    settings.battThreshold = threshold;
    String rstring = "Battery threshold set to "+String(threshold)+"V";
    strcpy(response, rstring.c_str());
}


void Command::init()
{
    parser.registerCommand("vs","", &sendVersion);
    parser.registerCommand("sf","su", &setFlow);
    parser.registerCommand("sm","si", &setFlowManual);
    parser.registerCommand("ss","u", &setSampleSet);
    parser.registerCommand("le","u", &enableLogFile);
    parser.registerCommand("pk","ssd", &setPIDk);
    parser.registerCommand("sks","", &showPIDks);
    parser.registerCommand("set","", &saveSettings);
    parser.registerCommand("rst","", &restoreSettings);
    parser.registerCommand("ps","u", &setPumpSpeed);
    parser.registerCommand("co2z","", &zeroCO2);
    parser.registerCommand("coz","", &zeroCO);
    parser.registerCommand("cos","d", &spanCO);
    parser.registerCommand("df","", &sendDataPacketFormat);
    parser.registerCommand("st","s", &setTime);
    parser.registerCommand("co2c","s", &send2co2);
    parser.registerCommand("co2p","u", &co2Pump);
    parser.registerCommand("fz","s", &flowZero);
    parser.registerCommand("fs","su", &flowSpan);
    parser.registerCommand("co2b","d", &co2background);
    parser.registerCommand("bt","d", &setBattThreshold);
}


