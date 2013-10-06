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
    : gpio_(func_),
      led_(gpio_),
      uart_(interruptMgr_, func_, SysClockFreq),
      uartSocket_(uart_, el_)
{
}

extern "C"
void interruptHandler()
{
//    System::instance().led().on();
    System::instance().interruptMgr().handleInterrupt();
}
