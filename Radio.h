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
        RadioPacket packetBuffer;
        void initHardware();

};

// pin assignments for LoRa 
/*  defines for Feather M0 with onblard LoRa
#define RFM95_CS    8
#define RFM95_INT   3
#define RFM95_RST   4
*/

// defines for M4 Express with radio Featherwing
#define RFM95_CS    9
#define RFM95_INT   11
#define RFM95_RST   6


#define RF95_FREQ 434.0

#endif