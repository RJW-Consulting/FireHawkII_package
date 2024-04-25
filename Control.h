#ifndef Control_h
#define Control_h

#include "Arduino.h"
#include <stdint.h>

#define CONTROL_STATE_IDLE 0
#define CONTROL_STATE_WAIT_TRIGGER 1
#define CONTROL_STATE_SAMPLE_0 2
#define CONTROL_STATE_SAMPLE_1 3
#define CONTROL_STATE_SAMPLE_2 4
#define CONTROL_STATE_SAMPLE_CO2_ZERO 5
#define CONTROL_STATE_SAMPLE_CO2_SPAN 6
#define CONTROL_STATE_SAMPLE_CO_ZERO 7
#define CONTROL_STATE_SAMPLE_CO_SPAN 8
#define CONTROL_STATE_SENDING_LOG 9



// TODO - OPTIONAL Revamp Control into automation
class Control
{
    public:
        void init();
        void tick();

        bool setSampleChannelFlow(uint8_t channel, uint16_t flowSCCM);
        bool setCO2ConcTrigger(float co2PPM);
        bool setSampleChangeCO2Mass(float co2mg);
        bool setSampleChangeTime(uint16_t seconds);
        bool startSampling();
        bool changeToNextSample();
        bool stopSampling();
        bool setValves(uint8_t set, bool state);
        bool zeroCO2();
        bool spanCO2(float co2PPM);
        bool zeroCO();
        bool spanCO(float coPPM);
        bool setSamplePump(bool state);
        bool setCO2Pump(bool state);
        bool setTime(String time);
        bool sendLogFile();
    
    private:
        char state;
        uint32_t stateSecs;

};

#endif
