//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "embxx/util/EventLoop.h"
#include "embxx/driver/Character.h"

#include "device/Function.h"
#include "device/Gpio.h"
#include "device/InterruptMgr.h"
#include "device/Timer.h"
#include "device/EventLoopDevices.h"
#include "device/Uart1.h"

#include "component/OnBoardLed.h"

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

    typedef component::OnBoardLed Led;

    static System& instance();

    inline EventLoop& eventLoop();
    inline InterruptMgr& interruptMgr();
    inline Uart& uart();
    inline UartSocket& uartSocket();
    inline Led& led();

private:
    System();

    EventLoop el_;

    // Devices
    InterruptMgr interruptMgr_;
    device::Function func_;
    device::Gpio gpio_;
    Uart uart_;

    // Drivers
    UartSocket uartSocket_;

    // Components
    Led led_;

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
System::Uart& System::uart()
{
    return uart_;
}

inline
System::UartSocket& System::uartSocket()
{
    return uartSocket_;
}

inline
System::Led& System::led()
{
    return led_;
}
