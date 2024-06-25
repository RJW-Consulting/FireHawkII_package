#include <Wire.h>
#include <String.h>
#include <WString.h>
#include "Driver_ppsystemsCO2.h"
#include "globals.h"

Driver_ppsystemsCO2::Driver_ppsystemsCO2()
{
  //Serial1.begin(baud); 
  /*
  while (!Serial1) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  */

  monitorInitialized = false;
 
}

void Driver_ppsystemsCO2::open(int baud, DateTime *now, uint8_t interval)
{
  Serial1.begin(baud);
  this->nowPtr = now;
  this->lastReadingTime = *(this->nowPtr);
  this->lastMessageTime = *(this->nowPtr);
  this->lastAccumulationTime = *(this->nowPtr);
  this->measInterval = interval;
  this->state = CO2_STATE_UNKNOWN;
  Serial.println("CO2 Driver Initialized");
}

void Driver_ppsystemsCO2::tick()
{
  // main gives time to driver to read from CO2 analyser, get measurements, read state, etc.
  // Determine if it's time to read from serial
  if (this->nowPtr->unixtime() > this->lastMessageTime.unixtime())
  {
    //Serial.println("Measurement time");
    receive(this->inText);
    int pos = inText.indexOf('\n');
    if (pos >= 0)
    {
      Serial.print("CO2>");
      Serial.println(inText);
      co2message = inText.substring(0,pos+1);
      inText = inText.substring(pos + 1);
    }
    if (co2message.length() > 0)
    {
      if (!receivedMeasurement())
      {
        if (!receivedZero())
        {
          if (!receivedWait())
          {
            this->state == '?';
            co2message = "";
            //Serial.println("Seeing nothing");
          }
        }
      }
      else
      {
        // Measurement received, plug into global
        readings.co2Conc = lastCO2Reading;
      }
    }
    readings.measurementTime = *(this->nowPtr);
    if (*(this->nowPtr) > lastAccumulationTime)
    {
      updateAccumulators();
      lastAccumulationTime = *nowPtr;
    }
    readings.co2Conc = lastCO2Reading;
    settings.co2State = state;
    if (!monitorInitialized)
      initMonitor();
  }
}

char Driver_ppsystemsCO2::getState()
{
  return this->state;
}

DateTime Driver_ppsystemsCO2::getMeasurementTime()
{
  return this->lastReadingTime;
}

bool Driver_ppsystemsCO2::receivedMeasurement()
{
  bool ret = false;
  //Serial.println(this->inText);
  size_t pos = this->co2message.indexOf("M ");
//  Serial.printf("pos = %d\n", pos);
  if (pos != -1)
  {
    size_t pos2 = this->co2message.indexOf('\n', pos);
    //Serial.printf("\npos2 = %d\n",pos2);
    if (pos2 > pos)
    {

       String measurementString = this->co2message.substring(pos,pos2);

      //Serial.print(measurementString);
      if (parseMeasurement(measurementString))
      {
        this->state = CO2_STATE_MEASURING;
        this->lastMessageTime = *(this->nowPtr);
        this->lastReadingTime = *(this->nowPtr);
        this->co2message = "";
        readings.co2Conc = lastCO2Reading;
        ret = true;
      }
    }
  }
  return ret;
}

bool Driver_ppsystemsCO2::parseMeasurement(String instr)
{
  size_t parts[9];
  int i;
  for (i=0; i < 9; i++)
  {
    size_t startAt = 0;
    if (i > 0)  startAt = parts[i-1]+1;
    parts[i] = instr.indexOf(' ', startAt);
    if (parts[i] == -1)
      break;
  }
  String lreading = instr.substring(parts[0]+1);
  this->lastCO2Reading = lreading.toFloat();
  //lreading = instr.substring(parts[3],parts[4]);
  //this->lastIRGATemperature = lreading.toFloat();
  this->lastReadingTime = *(this->nowPtr);
  return true;
}

bool Driver_ppsystemsCO2::receivedWait()
{
  bool ret = false;
  size_t pos;
  size_t pos2;

//  Serial.println(this->inText);
  pos = this->co2message.indexOf("W,");
//  Serial.printf("pos = %d\n", pos);
  if (pos != -1)
  {
    this->state = CO2_STATE_WARMUP;
    pos = this->co2message.indexOf(",");
    pos2 = this->co2message.indexOf("\n");
    this->lastIRGATemperature = this->co2message.substring(pos+1,pos2).toFloat();
    this->lastMessageTime = *(this->nowPtr);
    this->co2message = "";
    ret=true;
  }
  
  return false;
}

bool Driver_ppsystemsCO2::receivedZero()
{
  bool ret = false;
  size_t pos;
  size_t pos2;

//  Serial.println(this->inText);
  pos = this->co2message.indexOf("Z,");
//  Serial.printf("pos = %d\n", pos);
  if (pos != -1)
  {
    this->state = CO2_STATE_ZERO;
    pos = this->co2message.indexOf(",");
    pos2 = this->co2message.indexOf(" ");
    const char *countstr = this->co2message.substring(pos+1,pos2).c_str();
    this->zeroCount = this->co2message.substring(pos+1,pos2).toInt();
    pos = this->co2message.indexOf(" ",pos2+1);
    pos2 = this->co2message.indexOf("\n");
    this->zeroFinalCount = this->co2message.substring(pos+1,pos2).toInt();
    this->lastMessageTime = *(this->nowPtr);
    this->lastZeroTime = *(this->nowPtr);
    this->co2message = "";
    ret=true;
  }
  
  return false;
}

void Driver_ppsystemsCO2::startZero()
{
  this->send("Z");
}

int Driver_ppsystemsCO2::getZeroSec()
{
  return this->zeroCount;
}
    
int Driver_ppsystemsCO2::getZeroEndSec()
{
  return this->zeroFinalCount;
}

float Driver_ppsystemsCO2::getTemperature()
{
  return this->lastIRGATemperature;
}

void Driver_ppsystemsCO2::send(String message)
{
  if (message.length() == 1)
  {
    Serial1.print(message);
  }
  else
  {
    Serial1.print(message+"\r");
  }
}

 bool Driver_ppsystemsCO2::receiveAvailable()
 {
    return (Serial1.available() > 0);
 }

uint16_t Driver_ppsystemsCO2::receive(String &mess)
{
  int i = 0;
  while (Serial1.available() > 0) 
  {
    char inChar = (char)Serial1.read();
    mess += inChar;
    i++;
    if (inChar == '\n')
      break;
  }
  /*
  if (i > 0)
    Serial.print(message);
  else
     Serial.println("Nothing");
  */
  return i;

}

float Driver_ppsystemsCO2::getMeasurement(DateTime &measurementTime)
{
  measurementTime = this->lastReadingTime;
  return this->lastCO2Reading;
}

float Driver_ppsystemsCO2::calculateCO2Mass(float CO2_concentration_PPM, float flow_SCCM, float time_seconds, float temperature_Celsius, float pressure_millibars) {
    // Constants
    const float R = 8.314; // Ideal gas constant in J/(mol*K)
    const float M_CO2 = 44.01; // Molar mass of CO2 in g/mol

    // Convert temperature from Celsius to Kelvin
    float temperature_K = temperature_Celsius + 273.15;

    // Convert pressure from millibars to Pascals
    float pressure_Pa = pressure_millibars * 100;

    // Convert flow from SCCM to m^3/s
    float flow_m3s = flow_SCCM * 1e-6 / 60; // 1 SCCM = 1e-6 m^3/s

    // Convert CO2 concentration from PPM to fraction
    float CO2_concentration_fraction = CO2_concentration_PPM * 1e-6;

    // Calculate volume of CO2 collected using flow and time
    float volume_collected = flow_m3s * time_seconds;

    // Calculate moles of CO2 collected using ideal gas law
    float n_CO2 = (pressure_Pa * volume_collected) / (R * temperature_K);

    // Calculate mass of CO2 collected
    float mass_CO2 = n_CO2 * M_CO2;

    // return grams
    return mass_CO2;
}

void Driver_ppsystemsCO2::updateAccumulators()
{
  // get seconds since last reading
  // this must be called before lastMessageTime is updated
  if (readings.sampleSet > 0)
  {
    float seconds = (float) (this->nowPtr->unixtime() - lastAccumulationTime.unixtime());
    readings.co2MassGas += calculateCO2Mass(readings.co2Conc, readings.flowGas, seconds, readings.airTemp, readings.pressure);
    readings.co2MassOa += calculateCO2Mass(readings.co2Conc, readings.flowOa, seconds, readings.airTemp, readings.pressure);
    readings.co2MassAm += calculateCO2Mass(readings.co2Conc, readings.flowAm, seconds, readings.airTemp, readings.pressure);

  }

}

void Driver_ppsystemsCO2::resetAccumulators()
{
  readings.co2MassGas = 0;
  readings.co2MassOa = 0;
  readings.co2MassAm = 0;
  lastAccumulationTime = *nowPtr;
}

void Driver_ppsystemsCO2::setPump(bool state)
{
  if (state)
  {
    send("P1\n");
  }
  else
  {
    send("P0\n");
  }
  pumpRunning = state;
  settings.co2PumpOn = state;

}

bool Driver_ppsystemsCO2::getPumpState()
{
  return pumpRunning;
}
  
void Driver_ppsystemsCO2::initMonitor()
{
  // TODO-Fix CO2 monitor initialization
  send("S,11,1\n");  // reading every 1 second
  send("F0\n");      // readout CO2 only
  send("A0\n");     // turn off automatic zeros
  monitorInitialized = true;
}