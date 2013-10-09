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
#include "embxx/driver/TimerMgr.h"

#include "device/Function.h"
#include "device/Gpio.h"
#include "device/Led.h"
#include "device/InterruptMgr.h"
#include "device/Timer.h"
#include "device/EventLoopDevices.h"

class System
{
public:
    static const std::size_t EventLoopSpaceSize = 1024;
    typedef embxx::util::EventLoop<
        EventLoopSpaceSize,
        device::InterruptLock,
        device::WaitCond> EventLoop;

    typedef device::InterruptMgr<> InterruptMgr;
    typedef device::Timer<InterruptMgr> TimerDevice;

    static const std::size_t NumOfTimers = 10;
    typedef embxx::driver::TimerMgr<
        TimerDevice,
        EventLoop,
        NumOfTimers,
        embxx::util::StaticFunction<void (embxx::driver::ErrorStatus), 32> > TimerMgr;


    static System& instance();

    inline EventLoop& eventLoop();
    inline InterruptMgr& interruptMgr();
    inline TimerDevice& timerDevice();
    inline device::Led& led();
    inline TimerMgr& timerMgr();


private:
    System();

    EventLoop el_;
    InterruptMgr interruptMgr_;
    TimerDevice timerDevice_;
    device::Function func_;
    device::Gpio gpio_;
    device::Led led_;

    TimerMgr timerMgr_;
};

extern "C"
void interruptHandler();

// Implementation

inline
System::EventLoop& System::eventLoop()
{
    return el_;
}

inline
System::InterruptMgr& System::interruptMgr()
{
    return interruptMgr_;
}

inline
System::TimerDevice& System::timerDevice()
{
    return timerDevice_;
}

inline
device::Led& System::led()
{
    return led_;
}

inline
System::TimerMgr& System::timerMgr()
{
    return timerMgr_;
}

