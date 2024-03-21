#ifndef Driver_ProportionalValve_h
#define Driver_ProportionalValve_h

#include "Arduino.h"
#include <Wire.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <Adafruit_MCP4728.h>
#include <Adafruit_ADS1X15.h>
#include <TCA9548A-SOLDERED.h>
#include <PID_v1.h>
#include "globals.h"

#define FLOW_BUFFER_SIZE 0

#define ANALOG_FLOW_TABLE_SIZE 6

struct FlowData {
    double flow_sccm;
    double voltage_vdc;
};

class Driver_ProportionalValve
{
    public:
        Driver_ProportionalValve(
                    Adafruit_MCP4728 *imcp, 
                    TCA9548A *ii2cMux,
                    uint_fast8_t imuxChannel,
                    uint_fast8_t iflowI2CAddr,
                    Adafruit_ADS1115 *iflowADC,
                    uint_fast8_t iflowADCChannel, 
                    uint_fast8_t ivalveChannel,
                    double *isetpoint,
                    double *ireadback,
                    double *pKp,
                    double *pKi,
                    double *pKd,
                    char marker,
                    uint_fast8_t period);
        void init();
        void tick();
        void enablePID(bool doEnable){ enabled = doEnable;};
        void setManual(int manValveSetting){manualValveSetting = manValveSetting;};
        void setPWMpin(int pin){pwmPin = pin;};
        void updateKs();
        double getValveSetting(){return valveSetting;};

    private:
        void setValvePower(int power);
        int getFlow();
        int getAnalogFlow();
        double calculateFlowRate(double voltage, const std::vector<FlowData>& flowTable); 
        void addFlowPoint(int flow);
        void initFlowBuffer();
        int getMeanFlow();
        Adafruit_MCP4728 *mcp;
        TCA9548A *i2cMux;
        PID *pid;
        int flowBuffer[FLOW_BUFFER_SIZE];
        uint_fast8_t bufferHead;
        uint_fast8_t bufferTail;
        uint_fast8_t bufferSize;
        uint64_t bufferSum;
        uint_fast8_t muxChannel;
        uint_fast8_t flowI2CAddr;
        Adafruit_ADS1115 *flowADC;
        uint_fast8_t flowADCChannel; 
        uint_fast8_t valveChannel;
        double *setpoint;
        double *readback;
        double valveSetting = 0;
        double *kp;
        double *ki;
        double *kd;
        int manualValveSetting = -1;
        int pwmPin = -1;
        bool enabled = false;
        bool ksChanged = false;
        char which;
        uint_fast8_t pidPeriod;
        uint_fast8_t periodCount;
     
};

#define MIN_DAC 0
#define MAX_DAC 4095

#endif
