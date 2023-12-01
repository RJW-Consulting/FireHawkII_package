#include "DataLogger.h"
#include "globals.h"

DataLogger::DataLogger()
{
}

void DataLogger::init()
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
        if (currentFile)
        {
            currentFile.write(logLine.c_str());
            currentFile.flush();
            lastLogTime = *systemTime;
        }
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
    header += "flow_setpoint_sorbent,";
    header += "flow_sorbent,";
    header += "flow_setpoint_aerosol,";
    header += "flow_aerosol,";
    header += "flow_setpoint_carbon,";
    header += "flow_carbon,";
    header += "co2_status,";
    header += "co2_ppm,";
    header += "total_co2_sorbent,";
    header += "total_co2_aerosol,";
    header += "total_co2_carbon,";
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
    line += String(settings.flowSorbentSetPoint);
    line += ",";
    line += String(readings.flowSorbent);
    line += ",";
    line += String(settings.flowAerosolSetPoint);
    line += ",";
    line += String(readings.flowAerosol);
    line += ",";
    line += String(settings.flowCarbonSetPoint);
    line += ",";
    line += String(readings.flowCarbon);
    line += ",";
    line += settings.co2State;
    line += ",";
    line += co2Reading;
    line += ",";
    line += String(readings.co2MassSorbant,2);
    line += ",";
    line += String(readings.co2MassAerosol,2);
    line += ",";
    line += String(readings.co2MassCarbon,2);
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
    line += "*/\n";

    return line;
}

void DataLogger::fillDataPacket(struct DataPacket *packet)
{
    strcpy(packet->dateTime,"YYYYMMDDhhmmss");
    readings.measurementTime.toString(packet->dateTime);
    packet->sampleChannel = readings.sampleSet;
    packet->samplePump = settings.samplePumpOn;
    packet->co2Pump = settings.co2PumpOn;
    packet->flowSorbant = readings.flowSorbent;
    packet->flowAerosol = readings.flowAerosol;
    packet->flowCarbon = readings.flowCarbon;
    packet->co2Status = settings.co2State;
    packet->co2PPM = readings.co2Conc;
    packet->co2MassSorbant = readings.co2MassSorbant;
    packet->co2MassAerosol = readings.co2MassAerosol;
    packet->co2MassCarbon = readings.co2MassCarbon;
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





