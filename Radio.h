#ifndef Radio_h
#define Radio_h

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include "globals.h"

class Radio
{
    public:
        void init();
        void tick();

    private:
};

#endif