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
#include "embxx/io/InStreamBuf.h"
#include "embxx/io/OutStreamBuf.h"
#include "embxx/io/OutStream.h"
#include "embxx/device/DeviceOpQueue.h"
#include "embxx/device/IdDeviceCharAdapter.h"

#include "device/Function.h"
#include "device/Gpio.h"
#include "device/InterruptMgr.h"
#include "device/Timer.h"
#include "device/EventLoopDevices.h"
#include "device/Uart1.h"
#include "device/Spi0.h"

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

    typedef device::Gpio<InterruptMgr> Gpio;

    typedef device::Uart1<InterruptMgr> Uart;

    typedef device::Spi0<
        InterruptMgr,
        embxx::util::StaticFunction<void(), sizeof(void*) * 4>,
        embxx::util::StaticFunction<void(const embxx::error::ErrorStatus&)> > Spi;

    typedef embxx::device::DeviceOpQueue<Spi, 1> SpiOpQueue;

    typedef embxx::device::IdDeviceCharAdapter<SpiOpQueue> CharSpiAdapter;

    typedef embxx::driver::Character<Uart, EventLoop> UartDriver;

    typedef embxx::driver::Character<CharSpiAdapter, EventLoop> SpiDriver;

    typedef embxx::io::InStreamBuf<SpiDriver, 512> SpiInStreamBuf;

    typedef component::OnBoardLed<Gpio> Led;

    static const std::size_t LogStreamBufSize = 4096 * 2;
    typedef embxx::io::OutStreamBuf<UartDriver, LogStreamBufSize> LogStreamBuf;
    typedef embxx::io::OutStream<LogStreamBuf> OutStream;
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
    inline InterruptMgr& interruptMgr();

    inline Led& led();
    inline Log& log();

    SpiDriver& spi() {
        return spiDriver_;
    }

    SpiInStreamBuf& spiInBuf() {
        return spiInBuf_;
    }

    // TODO: move to private
    static const Spi::DeviceIdType SpiDevIdx = 0;
private:
    System();

    EventLoop el_;

    // Devices
    InterruptMgr interruptMgr_;
    device::Function func_;
    Gpio gpio_;
    Uart uart_;
    Spi spi_;
    SpiOpQueue spiOpQueue_;
    CharSpiAdapter spiCharAdapter_;

    // Drivers
    UartDriver uartDriver_;
    SpiDriver spiDriver_;

    // Components
    Led led_;
    LogStreamBuf buf_;
    OutStream stream_;
    Log log_;
    SpiInStreamBuf spiInBuf_;

    static const unsigned SysClockFreq = 250000000; // 250MHz
    static const unsigned InitialSpiFreq = 200000; // 200KHz

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
System::Led& System::led()
{
    return led_;
}

inline
System::Log& System::log()
{
    return log_;
}
