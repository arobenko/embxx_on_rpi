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
#include "embxx/driver/Generic.h"
#include "embxx/driver/TimerMgr.h"
#include "embxx/io/OutStreamBuf.h"
#include "embxx/io/InStreamBuf.h"

#include "device/Function.h"
#include "device/Gpio.h"
#include "device/InterruptMgr.h"
#include "device/Timer.h"
#include "device/EventLoopDevices.h"
#include "device/Uart1.h"

#include "component/OnBoardLed.h"
#include "component/Button.h"

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
    typedef embxx::driver::Generic<Gpio, EventLoop, void (bool)> ButtonDriver;
    typedef embxx::driver::Character<Uart, EventLoop> UartDriver;
    typedef embxx::driver::TimerMgr<
            TimerDevice,
            EventLoop,
            1> TimerMgr;

    // Components
    typedef component::OnBoardLed<Gpio> Led;
    typedef component::Button<
        ButtonDriver,
        false,
        embxx::util::StaticFunction<void(), sizeof(void*) * 4> > Button;
    typedef embxx::io::InStreamBuf<UartDriver, 1024> CommsInStreamBuf;
    typedef embxx::io::OutStreamBuf<UartDriver, 1024> CommsOutStreamBuf;

    static System& instance();

    inline EventLoop& eventLoop();
    inline InterruptMgr& interruptMgr();
    inline Gpio& gpio();
    inline Uart& uart();
    inline Led& led();
    inline Button& button();
    inline TimerDevice& timerDevice();
    inline TimerMgr& timerMgr();
    inline CommsInStreamBuf& commsInStreamBuf();
    inline CommsOutStreamBuf& commsOutStreamBuf();

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
    ButtonDriver buttonDriver_;
    UartDriver uartDriver_;
    TimerMgr timerMgr_;

    // Components
    Led led_;
    Button button_;
    CommsInStreamBuf commsInStreamBuf_;
    CommsOutStreamBuf commsOutStreamBuf_;

    static const unsigned SysClockFreq = 250000000; // 250MHz
    static const device::Function::PinIdxType ButtonPin = 23;
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
System::Button& System::button()
{
    return button_;
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
System::CommsInStreamBuf& System::commsInStreamBuf()
{
    return commsInStreamBuf_;
}

inline
System::CommsOutStreamBuf& System::commsOutStreamBuf()
{
    return commsOutStreamBuf_;
}

