#ifndef Driver_ppsystemsCO2_h
#define Driver_ppsystemsCO2_h

#include "Arduino.h" 
#include "RTClib.h"

class Driver_ppsystemsCO2
{
  public: 
  // TODO - implement CO2 monitor initialization
    Driver_ppsystemsCO2();
    void open(int baud, DateTime *now, uint8_t interval);
    void tick();
    char getState();
    void startZero();
    // TODO - OPTIONAL Implement CO2 Span/scale method
    int getZeroSec();
    int getZeroEndSec();
    float getMeasurement(DateTime &measurementTime);
    float getTemperature();
    DateTime getMeasurementTime();
    void send(String message);
    uint16_t receive(String &message);
    bool receiveAvailable();
    void setPump(bool state);
    bool getPumpState();
    void updateAccumulators();
    void resetAccumulators();
  
  private:
    DateTime *nowPtr;
    DateTime lastReadingTime;
    DateTime lastZeroTime;
    DateTime lastMessageTime;
    DateTime lastAccumulationTime;
    float lastCO2Reading;
    float lastIRGATemperature;
    int zeroCount;
    int zeroFinalCount;
    bool pumpRunning;
    uint8_t measInterval;
    String inText;
    String co2message;
    char state;

    bool receivedMeasurement();
    bool receivedWait();
    bool receivedZero();
    bool parseMeasurement(String instr);
    float calculateCO2Mass(float CO2_concentration_PPM, float flow_SCCM, float time_seconds, float temperature_Celsius, float pressure_millibars);


};

#define CO2_STATE_MEASURING   'M'
#define CO2_STATE_WARMUP      'W'
#define CO2_STATE_ZERO        'Z'
#define CO2_STATE_UNKNOWN     '?'

#endif