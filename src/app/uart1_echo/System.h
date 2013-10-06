//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//
#pragma once

#include "embxx/util/EventLoop.h"
#include "embxx/driver/Character.h"

#include "device/Function.h"
#include "device/Gpio.h"
#include "device/Led.h"
#include "device/InterruptMgr.h"
#include "device/Timer.h"
#include "device/EventLoopDevices.h"
#include "device/Uart1.h"

class System
{
public:
    static const std::size_t EventLoopSpaceSize = 1024;
    typedef embxx::util::EventLoop<
        EventLoopSpaceSize,
        device::InterruptLock,
        device::WaitCond> EventLoop;

    typedef device::InterruptMgr<> InterruptMgr;

    typedef device::Uart1<InterruptMgr> Uart;

    typedef embxx::driver::Character<Uart, EventLoop> UartSocket;

    static System& instance();

    inline EventLoop& eventLoop();
    inline InterruptMgr& interruptMgr();
    inline device::Led& led();
    inline Uart& uart();
    inline UartSocket& uartSocket();


private:
    System();

    EventLoop el_;
    InterruptMgr interruptMgr_;
    device::Function func_;
    device::Gpio gpio_;
    device::Led led_;
    Uart uart_;

    UartSocket uartSocket_;


    static const unsigned SysClockFreq = 250000000; // 250MHz
};

extern "C"
void interruptHandler();

// Implementation

inline
System::EventLoop& System::eventLoop()
{
    return el_;
}

inline System::InterruptMgr& System::interruptMgr()
{
    return interruptMgr_;
}

inline
device::Led& System::led()
{
    return led_;
}

inline
System::Uart& System::uart()
{
    return uart_;
}

inline
System::UartSocket& System::uartSocket()
{
    return uartSocket_;
}

