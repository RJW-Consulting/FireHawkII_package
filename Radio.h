#ifndef Radio_h
#define Radio_h

#include <arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include "globals.h"

class Radio
{
    public:
        void init();
        void tick();

    private:
        DataPacket packetBuffer;
        void initHardware();

};

// pin assignments for LoRa 
#define RFM95_CS    8
#define RFM95_INT   3
#define RFM95_RST   4

#endif