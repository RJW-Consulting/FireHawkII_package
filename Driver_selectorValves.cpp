
#include "Driver_selectorValves.h"
 
Driver_selectorValves::Driver_selectorValves()
{
}

void Driver_selectorValves::init()
{
    this->pca9685 = PCA9685();
    this->hitTime = 500;    // 500 ms for 1/2 second hit time
    this->hitDutyCycle = 100;
    this->holdDutyCycle = 50;
    this->initValveStates();
    this->doUpdate = false;
    pca9685.setupSingleDevice(Wire,PCA9685_BASE_I2C_ADDR);
    frequencyMin = pca9685.getFrequencyMin();
    frequencyMax = pca9685.getFrequencyMax();
    pca9685.setToFrequency(frequencyMax);   
}

void Driver_selectorValves::setMSClock(uint32_t *msClockPtr)
{
    msClock = msClockPtr;
}

void Driver_selectorValves::openValve(int gang, int valve)
{
    closeGang(gang);
    this->valveStates[gang][valve].state = this->hitDutyCycle;
    this->valveStates[gang][valve].openTime = *(this->msClock);
    doUpdate = true;
}

void Driver_selectorValves::closeGang(int gang)
{
    for (int valve = 0; valve<NUM_VALVES_PER_GANG; valve++)
    {
        this->valveStates[gang][valve].state = 0;
        this->valveStates[gang][valve].openTime = 0;
    }
    doUpdate = true;
}


void Driver_selectorValves::setHitTime(uint32_t  msecs)
{
    this->hitTime = msecs;
}


void Driver_selectorValves::setHoldPct(int pct)
{
    this->holdDutyCycle = pct;
}

void Driver_selectorValves::setHitPct(int pct)
{
    this->hitDutyCycle = pct;
}


uint8_t Driver_selectorValves::getvalveState(int gang, int valve)
{
    return this->valveStates[gang][valve].state;
}

void Driver_selectorValves::openSet(int set)
{
    for (int gang = 0; gang < NUM_VALVE_GANGS; gang++)
    {
        closeGang(gang);
        openValve(gang, set);
    }
    doUpdate = true;

}

void Driver_selectorValves::tick()
{
    int valve;

    for (int gang = 0; gang < NUM_VALVE_GANGS; gang++)
    {
        for (valve = 0; valve < NUM_VALVES_PER_GANG; valve++)
        {
            if ((valveStates[gang][valve].openTime) && 
                (*(msClock) - valveStates[gang][valve].openTime >= hitTime) && 
                (this->valveStates[gang][valve].state == hitDutyCycle))
            {
                this->valveStates[gang][valve].state = holdDutyCycle;
                doUpdate = true;
            }
        }
    }
    if (doUpdate)
    {
        updateValves();
    }
}

void Driver_selectorValves::initValveStates()
{
    int gang;
    int valve;

    for (gang = 0; gang < NUM_VALVE_GANGS; gang++)
    {
        for (valve = 0; valve < NUM_VALVES_PER_GANG; valve++)
        {
            this->valveStates[gang][valve].state = 0;
            this->valveStates[gang][valve].openTime = 0;
        }
    }
}

void Driver_selectorValves::updateValves()
{
    int gang = 0;
    int valve = 0;
    PCA9685::Channel channel = 0;

    for (gang = 0; gang < NUM_VALVE_GANGS; gang++)
    {
        for (valve = 0; valve < NUM_VALVES_PER_GANG; valve++)
        {
            channel = getDeviceChannel(gang, valve);

            pca9685.setDeviceChannelDutyCycle(PCA9685_BASE_I2C_ADDR, channel, valveStates[gang][valve].state);
        }
    }
}

PCA9685::Channel Driver_selectorValves::getDeviceChannel(int gang, int valve)
{
    return (gang * NUM_VALVES_PER_GANG) + valve;
}  




