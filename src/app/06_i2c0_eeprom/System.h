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
#include "device/I2C0.h"

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

    typedef device::I2C0<
        InterruptMgr,
        embxx::util::StaticFunction<void(), 20>,
        embxx::util::StaticFunction<void(const embxx::error::ErrorStatus&), 20> > I2C;

    typedef embxx::device::DeviceOpQueue<I2C, 2> I2cOpQueue;

    typedef embxx::device::IdDeviceCharAdapter<I2cOpQueue> CharI2cAdapter;

//    typedef embxx::driver::Character<Uart, EventLoop> UartDriver;

    typedef embxx::driver::Character<CharI2cAdapter, EventLoop> I2cDriver;

    typedef component::OnBoardLed<Gpio> Led;

//    static const std::size_t OutStreamBufSize = 1024;
//    typedef embxx::io::OutStreamBuf<UartDriver, OutStreamBufSize> OutStreamBuf;
//    typedef embxx::io::OutStream<OutStreamBuf> OutStream;
//    typedef embxx::util::log::StreamFlushSuffixer<
//            embxx::util::log::StreamableValueSuffixer<
//                const OutStream::CharType*,
//                embxx::util::log::LevelStringPrefixer<
//                    embxx::util::StreamLogger<
//                        embxx::util::log::Debug,
//                        OutStream
//                    >
//                >
//            >
//        > Log;

    static System& instance();

    inline EventLoop& eventLoop();
    inline InterruptMgr& interruptMgr();
    inline Uart& uart();
//    inline I2C& i2c();
    inline I2cDriver& i2cDriver();
    inline Led& led();
//    inline Log& log();

private:
    System();

    EventLoop el_;

    // Devices
    InterruptMgr interruptMgr_;
    device::Function func_;
    Gpio gpio_;
//    Uart uart_;
    I2C i2c_;
    I2cOpQueue i2cOpQueue_;
    CharI2cAdapter i2cCharAdapter_;

    // Drivers
//    UartDriver uartDriver_;
    I2cDriver i2cDriver_;

    // Components
    Led led_;
//    OutStreamBuf buf_;
//    OutStream stream_;
//    Log log_;

    static const unsigned SysClockFreq = 250000000; // 250MHz
    static const unsigned I2cFreq = 100000; // 100KHz
    static const I2C::DeviceIdType EepromAddress = 0x55;
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

//inline
//System::Uart& System::uart()
//{
//    return uart_;
//}

//inline
//System::I2C& System::i2c()
//{
//    return i2c_;
//}

inline
System::I2cDriver& System::i2cDriver()
{
    return i2cDriver_;
}

inline
System::Led& System::led()
{
    return led_;
}

//inline
//System::Log& System::log()
//{
//    return log_;
//}
