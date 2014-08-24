//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
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
#include "embxx/driver/Generic.h"
#include "embxx/driver/TimerMgr.h"
#include "embxx/io/InStreamBuf.h"

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

    // Devices
    typedef device::InterruptMgr<> InterruptMgr;
    typedef device::Gpio<InterruptMgr, 1U> Gpio;
    typedef device::Uart1<InterruptMgr> Uart;
    typedef device::Timer<InterruptMgr> TimerDevice;

    // Drivers
    struct OutCharacterTraits
    {
        typedef embxx::util::StaticFunction<void(const embxx::error::ErrorStatus&, std::size_t)> ReadHandler;
        typedef std::nullptr_t WriteHandler;
        typedef std::nullptr_t ReadUntilPred; // no read until
        static const std::size_t ReadQueueSize = 1;
        static const std::size_t WriteQueueSize = 0; // no write support
    };
    typedef embxx::driver::Character<Uart, EventLoop, OutCharacterTraits> UartDriver;
    typedef embxx::driver::TimerMgr<
            TimerDevice,
            EventLoop,
            1> TimerMgr;

    // Components
    typedef component::OnBoardLed<Gpio> Led;
    typedef embxx::io::InStreamBuf<UartDriver, 1024> InStreamBuf;

    static System& instance();

    inline EventLoop& eventLoop();
    inline InterruptMgr& interruptMgr();
    inline Gpio& gpio();
    inline Uart& uart();
    inline Led& led();
    inline TimerDevice& timerDevice();
    inline TimerMgr& timerMgr();
    inline InStreamBuf& inBuf();

private:
    System();

    EventLoop el_;

    // Devices
    InterruptMgr interruptMgr_;
    device::Function func_;
    Gpio gpio_;
    Uart uart_;
    TimerDevice timerDevice_;

    // Drivers
    UartDriver uartDriver_;
    TimerMgr timerMgr_;

    // Components
    Led led_;
    InStreamBuf inBuf_;

    static const unsigned SysClockFreq = 250000000; // 250MHz
    static const unsigned UartBaud = 115200;
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
System::Gpio& System::gpio()
{
    return gpio_;
}

inline
System::Uart& System::uart()
{
    return uart_;
}

inline
System::Led& System::led()
{
    return led_;
}

inline
System::TimerDevice& System::timerDevice()
{
    return timerDevice_;
}

inline
System::TimerMgr& System::timerMgr()
{
    return timerMgr_;
}

inline
System::InStreamBuf& System::inBuf()
{
    return inBuf_;
}

