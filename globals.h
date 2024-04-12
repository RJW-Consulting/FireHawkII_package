#ifndef globals_h
#define globals_h

#include "RTClib.h"
#include "FreeRTOS.h"
#include <semphr.h>

#define NUM_VALVE_GANGS 3
#define NUM_VALVES_PER_GANG 3

// defines for radio queue sizes
#define RADIO_DATA_QUEUE_NUM_RECORDS 4
#define RADIO_COMMAND_QUEUE_NUM_RECORDS 4
#define RADIO_COMMAND_QUEUE_RECORD_SIZE 60
#define RADIO_COMMAND_RESPONSE_QUEUE_NUM_RECORDS 2
#define RADIO_COMMAND_RESPONSE_QUEUE_RECORD_SIZE 128

struct Readings {
    DateTime measurementTime;
    float co2Conc;
    float co2MassGas;
    float co2MassOa;
    float co2MassAm;
    float coConc;
    float coMv;
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
    bool baseStationAnswering;
    uint samplePumpSpeed;
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
    char packetType;
    char dateTime[15];
    uint8_t sampleChannel;
    uint8_t samplePump;
    uint8_t co2Pump;
    uint16_t flowSorbant;
    uint16_t flowOa;
    uint16_t flowAm;
    char co2Status;
    float co2PPM;
    float co2MassGas;
    float co2MassOa;
    float co2MassAm;
    float coMv;
    float coConc;
    float pressure;
    float rh;
    float airTemp;
    float caseTemp;
    float batteryV;
    float pressure1;
    float pressure2;
} __attribute__((__packed__));

struct StringPacket{
    char packetType;
    char chars[RADIO_COMMAND_RESPONSE_QUEUE_RECORD_SIZE];
} __attribute__((__packed__));

union RadioPacket{
    struct DataPacket dataPacket;
    struct StringPacket stringPacket;
};

extern struct Readings readings;
extern struct Settings settings;

extern SemaphoreHandle_t  i2cMutex;
extern QueueHandle_t handle_command_queue;
extern QueueHandle_t handle_command_response_queue;
extern QueueHandle_t handle_data_queue;

#define MAXFLOW_GAS 1000
#define MAXFLOW_OA  6000
#define MAXFLOW_AM  6000

// Required for Serial on Zero based boards
//#define Serial SERIAL_PORT_USBVIRTUAL


#endif