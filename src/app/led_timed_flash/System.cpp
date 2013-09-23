//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#include "System.h"

System& System::instance()
{
    static System system;
    return system;
}

System::System()
    : timerDevice_(interruptMgr_),
      gpio_(func_),
      led_(gpio_),
      timerMgr_(timerDevice_, el_)
{
}

extern "C"
void interruptHandler()
{
    System::instance().interruptMgr().handleInterrupt();
}
