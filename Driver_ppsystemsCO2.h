#ifndef Driver_ppsystemsCO2_h
#define Driver_ppsystemsCO2_h

#include "Arduino.h" 
#include "RTClib.h"

class Driver_ppsystemsCO2
{
  public: 
    Driver_ppsystemsCO2();
    void open(int baud, DateTime *now, uint8_t interval);
    void tick();
    char getState();
    void startZero();
    int getZeroSec();
    int getZeroEndSec();
    float getMeasurement(DateTime &measurementTime);
    float getTemperature();
    DateTime getMeasurementTime();
    void send(String message);
    uint16_t receive(String &message);
    bool receiveAvailable();
  
  private:
    DateTime *nowPtr;
    DateTime lastReadingTime;
    DateTime lastZeroTime;
    DateTime lastMessageTime;
    float lastCO2Reading;
    float lastIRGATemperature;
    int zeroCount;
    int zeroFinalCount;
    uint8_t measInterval;
    String inText;
    String co2message;
    char state;

    bool receivedMeasurement();
    bool receivedWait();
    bool receivedZero();
    bool parseMeasurement(String instr);

};

#define CO2_STATE_MEASURING   'M'
#define CO2_STATE_WARMUP      'W'
#define CO2_STATE_ZERO        'Z'
#define CO2_STATE_UNKNOWN     '?'

#endif