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
#include "component/Eeprom.h"

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
        embxx::util::StaticFunction<void(), sizeof(void*) * 4>,
        embxx::util::StaticFunction<void(const embxx::error::ErrorStatus&), sizeof(void*) * 4> > I2C;

    typedef embxx::device::DeviceOpQueue<I2C, 2> I2cOpQueue;

    typedef embxx::device::IdDeviceCharAdapter<I2cOpQueue> CharI2cAdapter;

    typedef embxx::driver::Character<Uart, EventLoop> UartDriver;

    typedef embxx::driver::Character<CharI2cAdapter, EventLoop> I2cDriver;

    typedef component::OnBoardLed<Gpio> Led;
    typedef component::Eeprom<
        I2cDriver,
        embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&, std::size_t), sizeof(void*) * 6>
    > Eeprom;

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
    inline InterruptMgr& interruptMgr();

    inline Led& led();
    inline Eeprom& eeprom1();
    inline Eeprom& eeprom2();
    inline Log& log();

private:
    System();

    EventLoop el_;

    // Devices
    InterruptMgr interruptMgr_;
    device::Function func_;
    Gpio gpio_;
    Uart uart_;
    I2C i2c_;
    I2cOpQueue i2cOpQueue_;
    CharI2cAdapter i2cCharAdapter1_;
    CharI2cAdapter i2cCharAdapter2_;

    // Drivers
    UartDriver uartDriver_;
    I2cDriver i2cDriver1_;
    I2cDriver i2cDriver2_;

    // Components
    Led led_;
    Eeprom eeprom1_;
    Eeprom eeprom2_;
    OutStreamBuf buf_;
    OutStream stream_;
    Log log_;

    static const unsigned SysClockFreq = 250000000; // 250MHz
    static const unsigned I2cFreq = 100000; // 100KHz
    static const I2C::DeviceIdType EepromAddress1 = 0x54;
    static const I2C::DeviceIdType EepromAddress2 = 0x55;
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

inline System::Eeprom& System::eeprom1()
{
    return eeprom1_;
}

inline System::Eeprom& System::eeprom2()
{
    return eeprom2_;
}

inline
System::Log& System::log()
{
    return log_;
}
