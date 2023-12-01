#ifndef Driver_mprls_h
#define Driver_mprls_h

#include <Wire.h>
#include <stdint.h>
#include <Adafruit_MPRLS.h>
#include <Adafruit_MCP4728.h>
#include <TCA9548A-SOLDERED.h>

// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin

class PressureSensor
{
    public:
        PressureSensor(TCA9548A *mux, int mChannel, float *pVariable);
        void init();
        float getPressure();
        void tick();

    private:
        TCA9548A *i2cMux;
        int muxChannel;
        Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);
        float *pVar;

};


#endif
