#ifndef globals_h
#define globals_h

#include "RTClib.h"
#include "FreeRTOS_SAMD21.h"
#include <semphr.h>

#define NUM_VALVE_GANGS 3
#define NUM_VALVES_PER_GANG 3

struct Readings {
    DateTime measurementTime;
    float co2Conc;
    float co2MassGas;
    float co2MassOa;
    float co2MassAm;
    float coConc;
    uint coMv;
    float pressure;
    float rh;
    float airTemp;
    float caseTemp;
    float batteryV;
    uint sampleSet;
    double flowGas; 
    double flowOa; 
    double flowAm; 
    float pressure1;
    float pressure2;
};

struct Settings {
    bool valveOpen[NUM_VALVE_GANGS][NUM_VALVES_PER_GANG];
    bool autoSample;
    bool autoSampleWaiting;
    bool autoSampleCollecting;
    bool samplePumpOn;
    bool co2PumpOn;
    uint stationRadioAddress;
    uint droneRadioAddress;
    char co2State;
    double flowGasSetPoint; 
    double flowOaSetPoint; 
    double flowAmSetPoint;
    double gasPIDKp;    
    double gasPIDKi;
    double gasPIDKd;
    double oaPIDKp;    
    double oaPIDKi;
    double oaPIDKd;
    double amPIDKp;    
    double amPIDKi;
    double amPIDKd;
    float coCalLowConc;
    float coCalLowMV;
    float coCalHighConc;
    float coCalHighMV; 
};

struct DataPacket{
    char dateTime[16];
    uint_fast8_t sampleChannel;
    uint_fast8_t samplePump;
    uint_fast8_t co2Pump;
    uint_fast16_t flowSorbant;
    uint_fast16_t flowOa;
    uint_fast16_t flowAm;
    char co2Status;
    float co2PPM;
    float co2MassGas;
    float co2MassOa;
    float co2MassAm;
    uint coMv;
    float coConc;
    float pressure;
    float rh;
    float airTemp;
    float caseTemp;
    float batteryV;
    float pressure1;
    float pressure2;
};

extern struct Readings readings;
extern struct Settings settings;

extern SemaphoreHandle_t  i2cMutex;
extern QueueHandle_t handle_command_queue;
extern QueueHandle_t handle_data_queue;

#endif