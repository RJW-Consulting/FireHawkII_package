#include "DataLogger.h"
#include "globals.h"

DataLogger::DataLogger()
{
}

void DataLogger::init()
{
    initSDCard();
    loadSettings();
}

bool DataLogger::initSDCard()
{
    bool ret = false;
    logFileName = "";
    if (!SD.begin(SD_CARD_CS)) 
        Serial.println("Card failed, or not present");
    else
    {
        Serial.println("card initialized.");
        ret = true;
    }
    logInterval = 1;
    return ret;
}

char section[] = "settings";

void DataLogger::loadSettings()
{
    bool wasLogging = false;
    if (logFileName != "")
    {
        stopLogging();
        wasLogging = true;
    }
    minIni ini("settings.ini");
    settings.droneRadioAddress = ini.getl(section, "droneRadioAddress", 1);
    settings.stationRadioAddress = ini.getl(section, "stationRadioAddress", 0);
    settings.flowGasSetPoint = ini.getl(section, "flowGasSetPoint", 1); 
    settings.flowOaSetPoint = ini.getl(section, "flowOaSetPoint", 1); 
    settings.flowAmSetPoint = ini.getl(section, "flowAmSetPoint", 1);
    settings.samplePumpSpeed = ini.getl(section, "samplePumpSpeed", 100);
    settings.gasPIDKp = ini.getf(section, "gasPIDKp", 0.01);    
    settings.gasPIDKi = ini.getf(section, "gasPIDKi", 0.5);
    settings.gasPIDKd = ini.getf(section, "gasPIDKd", 0.01);
    settings.oaPIDKp = ini.getf(section, "oaPIDKp", 0.01);    
    settings.oaPIDKi = ini.getf(section, "oaPIDKi", 0.5);
    settings.oaPIDKd = ini.getf(section, "oaPIDKd", 0.01);
    settings.amPIDKp = ini.getf(section, "amPIDKp", 0.01);    
    settings.amPIDKi = ini.getf(section, "amPIDKi", 0.5);
    settings.amPIDKd = ini.getf(section, "amPIDKd", 0.01);

    if (wasLogging)
    {
        beginLogging();
    }
}

bool DataLogger::saveSettings()
{
    bool wasLogging = false;
    if (logFileName != "")
    {
        stopLogging();
        wasLogging = true;
    }
    minIni ini("settings.ini");
    bool success = true;
    success &= ini.put(section, "droneRadioAddress", (int) settings.droneRadioAddress);
    success &= ini.put(section, "stationRadioAddress", (int) settings.stationRadioAddress);
    success &= ini.put(section, "flowGasSetPoint", (long) settings.flowGasSetPoint); 
    success &= ini.put(section, "flowOaSetPoint", (long) settings.flowOaSetPoint); 
    success &= ini.put(section, "flowAmSetPoint", (long) settings.flowAmSetPoint);
    success &= ini.put(section, "samplePumpSpeed", (int) settings.samplePumpSpeed);
    success &= ini.put(section, "gasPIDKp", (INI_REAL) settings.gasPIDKp);    
    success &= ini.put(section, "gasPIDKi", (INI_REAL) settings.gasPIDKi);
    success &= ini.put(section, "gasPIDKd", (INI_REAL)settings.gasPIDKd);
    success &= ini.put(section, "oaPIDKp", (INI_REAL) settings.oaPIDKp);    
    success &= ini.put(section, "oaPIDKi", (INI_REAL) settings.oaPIDKi);
    success &= ini.put(section, "oaPIDKd", (INI_REAL) settings.oaPIDKd);
    success &= ini.put(section, "amPIDKp", (INI_REAL) settings.amPIDKp);    
    success &= ini.put(section, "amPIDKi", (INI_REAL) settings.amPIDKi);
    success &= ini.put(section, "amPIDKd", (INI_REAL) settings.amPIDKd);

    if (wasLogging)
    {
        beginLogging();
    }

    return success;
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

    if (initSDCard())
    {
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
    logFileName = "";
    return true;
}


void DataLogger::tick()
{
    if ((systemTime->unixtime() - lastLogTime.unixtime()) >= logInterval)
    {
        String logLine = buildLogString();
        if (Serial.availableForWrite())
            Serial.println(logLine);

        // Send data to radio data queue
        fillDataPacket(&packet);
        xQueueSend(handle_data_queue, &packet, portMAX_DELAY);

        if (logFileName != "")
        {
            currentFile.write(logLine.c_str());
            currentFile.flush();
        }
        lastLogTime = *systemTime;
    } 
}

String DataLogger::getLogStringHeader()
{
    String header;
    
    header = "";
    header += "date_time,";
    header += "sample_channel,";
    header += "sample_pump,";
    header += "co2_pump,";
    header += "flow_setpoint_gas,";
    header += "flow_gas,";
    header += "flow_setpoint_oa,";
    header += "flow_oa,";
    header += "flow_setpoint_am,";
    header += "flow_am,";
    header += "co2_status,";
    header += "co2_ppm,";
    header += "total_co2_gas,";
    header += "total_co2_oa,";
    header += "total_co2_am,";
    header += "co_mv,";
    header += "co_ppm,";
    header += "air_t,";
    header += "air_p,";
    header += "air_rh,";
    header += "case_t,";
    header += "battery_v,";
    header += "pressure1,";
    header += "pressure2,";
    return header;
}

String DataLogger::logicalString(bool val)
{
    String line;
    if (val)
        line = "1";
    else
        line = "0";
    return line;
}

String DataLogger::buildLogString()
{
    String line = "";
    String co2Reading;
    if (settings.co2State == 'M')
    {
        co2Reading = String(readings.coConc,2);
    }
    else
    {
        co2Reading += "***";
    }
    line = "/*"; 
    line += readings.measurementTime.timestamp();
    line += ",";
    line += String(readings.sampleSet);
    line += ",";
    line += logicalString(settings.samplePumpOn);
    line += ",";
    line += logicalString(settings.co2PumpOn);
    line += ",";
    line += String(settings.flowGasSetPoint);
    line += ",";
    line += String(readings.flowGas);
    line += ",";
    line += String(settings.flowOaSetPoint);
    line += ",";
    line += String(readings.flowOa);
    line += ",";
    line += String(settings.flowAmSetPoint);
    line += ",";
    line += String(readings.flowAm);
    line += ",";
    line += settings.co2State;
    line += ",";
    line += co2Reading;
    line += ",";
    line += String(readings.co2MassGas,2);
    line += ",";
    line += String(readings.co2MassOa,2);
    line += ",";
    line += String(readings.co2MassAm,2);
    line += ",";
    line += String(readings.coMv);
    line += ",";
    line += String(readings.coConc,2);
    line += ",";
    line += String(readings.airTemp,1);
    line += ",";
    line += String(readings.pressure,1);
    line += ",";
    line += String(readings.rh,1);
    line += ",";
    line += String(readings.caseTemp,1);
    line += ",";
    line += String(readings.batteryV,2);
    line += ",";
    line += String(readings.pressure1,2);
    line += ",";
    line += String(readings.pressure2,2);
    line += ",";
    line += String(gasValve.getValveSetting());
    line += ",";
    line += String(oaValve.getValveSetting());
    line += ",";
    line += String(amValve.getValveSetting());
    line += "*/\n";

    return line;
}

uint16_t DataLogger::dataPacketSize()
{
    return sizeof(DataPacket);
}

void DataLogger::fillDataPacket(struct DataPacket *packet)
{
    strcpy(packet->dateTime,"YYYYMMDDhhmmss");
    readings.measurementTime.toString(packet->dateTime);
    packet->sampleChannel = readings.sampleSet;
    packet->samplePump = settings.samplePumpOn;
    packet->co2Pump = settings.co2PumpOn;
    packet->flowSorbant = readings.flowGas;
    packet->flowOa = readings.flowOa;
    packet->flowAm = readings.flowAm;
    packet->co2Status = settings.co2State;
    packet->co2PPM = readings.co2Conc;
    packet->co2MassGas = readings.co2MassGas;
    packet->co2MassOa = readings.co2MassOa;
    packet->co2MassAm = readings.co2MassAm;
    packet->coMv = readings.coMv;
    packet->coConc = readings.coConc;
    packet->pressure = readings.pressure;
    packet->rh = readings.rh;
    packet->airTemp = readings.airTemp;
    packet->caseTemp = readings.caseTemp;
    packet->batteryV = readings.batteryV;
    packet->pressure1 = readings.pressure1;
    packet->pressure2 = readings.pressure2;
}





