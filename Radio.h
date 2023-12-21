#ifndef Radio_h
#define Radio_h

#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include "globals.h"

class Radio
{
    public:
        Radio();
        void init();
        void tick();

    private:
};

#endif