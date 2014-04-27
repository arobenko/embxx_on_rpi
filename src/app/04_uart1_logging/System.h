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
#include "embxx/util/StreamLogger.h"
#include "embxx/util/log/LevelStringPrefixer.h"
#include "embxx/util/log/StreamableValueSuffixer.h"
#include "embxx/util/log/StreamFlushSuffixer.h"
#include "embxx/driver/Character.h"
#include "embxx/driver/TimerMgr.h"
#include "embxx/io/OutStreamBuf.h"
#include "embxx/io/OutStream.h"

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
    typedef device::Gpio<InterruptMgr> Gpio;
    typedef device::Uart1<InterruptMgr> Uart;
    typedef device::Timer<InterruptMgr> TimerDevice;

    // Drivers
    struct CharacterTraits
    {
        typedef std::nullptr_t ReadHandler;
        typedef embxx::util::StaticFunction<void(const embxx::error::ErrorStatus&, std::size_t)> WriteHandler;
        typedef std::nullptr_t ReadUntilPred;
        static const std::size_t ReadQueueSize = 0;
        static const std::size_t WriteQueueSize = 1;
    };
    typedef embxx::driver::Character<Uart, EventLoop, CharacterTraits> UartDriver;
    typedef embxx::driver::TimerMgr<
            TimerDevice,
            EventLoop,
            1> TimerMgr;


    // Components
    typedef component::OnBoardLed<Gpio> Led;
    static const std::size_t OutStreamBufSize = 1024;
    typedef embxx::io::OutStreamBuf<UartDriver, OutStreamBufSize> OutStreamBuf;
    typedef embxx::io::OutStream<OutStreamBuf> OutStream;
    typedef embxx::util::log::StreamFlushSuffixer<
            embxx::util::log::StreamableValueSuffixer<
                const OutStream::CharType*,
                embxx::util::log::LevelStringPrefixer<
                    embxx::util::StreamLogger<
                        embxx::util::log::Debug,
                        OutStream
                    >
                >
            >
        > Log;

    static System& instance();
    inline EventLoop& eventLoop();

    // Devices
    inline InterruptMgr& interruptMgr();
    inline Uart& uart();

    // Drivers
    inline TimerMgr& timerMgr();

    // Components
    inline Led& led();
    inline Log& log();

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
    OutStreamBuf buf_;
    OutStream stream_;
    Log log_;

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

inline System::Uart& System::uart()
{
    return uart_;
}

inline System::TimerMgr& System::timerMgr()
{
    return timerMgr_;
}

inline
System::Led& System::led()
{
    return led_;
}

inline
System::Log& System::log()
{
    return log_;
}

