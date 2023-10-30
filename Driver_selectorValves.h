#ifndef Driver_selectorValves_h
#define Driver_selectorValves_h

#include "Arduino.h" 
#include "PCA9685.h"

#define NUM_VALVE_GANGS 3
#define NUM_VALVES_PER_GANG 3

//#define PCA9685_BASE_I2C_ADDR 0x40
#define PCA9685_BASE_I2C_ADDR 0x40

struct SelValveState {
    PCA9685::Percent state;
    uint32_t openTime;
};

class Driver_selectorValves
{
  public: 
    Driver_selectorValves();
    void init();
    void setMSClock(uint32_t *msClockPtr);
    void openValve(int gang, int valve);
    void closeGang(int gang);
    void setHitTime(uint32_t  msecs);
    void setHitPct(int pct);
    void setHoldPct(int pct);
    uint8_t getvalveState(int gang, int valve);
    void tick();
  
  private:
    uint32_t hitTime;
    uint32_t *msClock;
    struct SelValveState valveStates[NUM_VALVE_GANGS][NUM_VALVES_PER_GANG];
    void initValveStates();
    void updateValves();
    bool doUpdate;
    PCA9685::Channel getDeviceChannel(int gang, int valve);
    
    uint32_t padding0;
    uint32_t padding1;
    PCA9685::Percent hitDutyCycle;
    PCA9685::Percent holdDutyCycle;
    PCA9685::Frequency frequencyMin;
    PCA9685::Frequency frequencyMax;
    PCA9685 pca9685;
 
};


#endif