#ifndef Command_h
#define Command_h

#include "FreeRTOS_SAMD21.h"

class Command
{
    public:
        void init();
        void tick();

    private:
        void checkAndParseCommand();

};

#endif
