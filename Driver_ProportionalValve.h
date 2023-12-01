#ifndef Driver_ProportionalValve_h
#define Driver_ProportionalValve_h

#include "Arduino.h"
#include <Wire.h>
#include <stdint.h>
#include <Adafruit_MCP4728.h>
#include <TCA9548A-SOLDERED.h>
#include <PID_v1.h>

class Driver_ProportionalValve
{
    public:
        Driver_ProportionalValve(Adafruit_MCP4728 *imcp, 
                    TCA9548A *ii2cMux,
                    uint_fast8_t imuxChannel,
                    uint_fast8_t iflowI2CAddr, 
                    uint_fast8_t ivalveChannel,
                    double *isetpoint,
                    double *ireadback);
        void tick();
        void enablePID(bool doEnable){ enabled = doEnable;};
        void setManual(int manValveSetting){manualValveSetting = manValveSetting;};
        void setKp(double ikp){kp = ikp;}; 
        void setKi(double iki){ki = iki;}; 
        void setKd(double ikd){kd = ikd;}; 
        void setPWMpin(int pin){pwmPin = pin;};

    private:
        void setValvePower(int power);
        int getFlow();
        Adafruit_MCP4728 *mcp;
        TCA9548A *i2cMux;
        PID *pid;
        uint_fast8_t muxChannel;
        uint_fast8_t flowI2CAddr;
        uint_fast8_t valveChannel;       
        double *setpoint;
        double *readback;
        double valveSetting = 0;
        double kp = 0.01;
        double ki = 0.5;
        double kd = 0.01;
        int manualValveSetting = -1;
        int pwmPin = -1;
        bool enabled = false;
     
};

#define MIN_DAC 0
#define MAX_DAC 4095

#endif
