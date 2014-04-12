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

#include "System.h"

#include <functional>
#include <cstdint>
#include <array>

#include "embxx/util/Assert.h"
#include "embxx/util/StreamLogger.h"
#include "embxx/io/access.h"

namespace
{

class LedOnAssert : public embxx::util::Assert
{
public:
    typedef System::Led Led;
    LedOnAssert(Led& led)
        : led_(led)
    {
    }

    virtual void fail(
        const char* expr,
        const char* file,
        unsigned int line,
        const char* function)
    {
        static_cast<void>(expr);
        static_cast<void>(file);
        static_cast<void>(line);
        static_cast<void>(function);

        led_.on();
//        device::interrupt::disable();
//        while (true) {;}

        auto& log = System::instance().log();
        SLOG(log, embxx::util::log::Error, "Assertion `" << expr << "` in " << function << " at " << file << ":" << line);
    }

private:
    Led& led_;
};

void performInit(unsigned attempt)
{
    static const std::uint8_t Cmd0[] = {
        0x40, 0x0, 0x0, 0x0, 0x0, 0x95
    };

    static const std::size_t Cmd0Size = std::extent<decltype(Cmd0)>::value;

    SLOG(System::instance().log(), embxx::util::log::Info, "Attempt " << attempt);

    auto& spi = System::instance().spi();
    spi.asyncWrite(
        &Cmd0[0],
        Cmd0Size,
        [](const embxx::error::ErrorStatus& es, std::size_t bytesWritten)
        {
            SLOG(System::instance().log(), embxx::util::log::Info, "Write complete!");
            GASSERT(!es);
            GASSERT(bytesWritten == Cmd0Size);
        });

    auto& spiInBuf = System::instance().spiInBuf();
    spiInBuf.start();

    static const unsigned DataBufSize = 32;
    spiInBuf.asyncWaitDataAvailable(
        DataBufSize,
        [&spiInBuf, attempt](const embxx::error::ErrorStatus& es)
        {
            SLOG(System::instance().log(), embxx::util::log::Info, "Block");
            GASSERT(!es);
            spiInBuf.stop();
            auto iter = spiInBuf.begin();
            for (auto i = 0U; i < DataBufSize; ++i) {
                auto byte = embxx::io::readBig<std::uint8_t>(iter);
                if (byte != 0xff) {
                    SLOG(System::instance().log(), embxx::util::log::Info, "Byte=0x" << embxx::io::hex << (unsigned)byte);
                    return;
                }
            }
            spiInBuf.consume();

            for (volatile int i = 0; i < 1000000; ++i) {}
            performInit(attempt + 1);
        });
}

}  // namespace

int main() {
    device::interrupt::disable();

    auto& system = System::instance();
    auto& led = system.led();

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& log = system.log();
    SLOG(log, embxx::util::log::Info, "Starting Write...");

    led.on();
    for (volatile int i = 0; i < 5000000; ++i) {}
    led.off();

//    auto& spi = system.spi();
//    spi.device().device().device().setFillChar(0xff);

    performInit(0);

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    // TODO: problems:
    // 1. Need to wait 1ms before sending
    // 2. Removing CS every 16

    GASSERT(0); // Mustn't exit
	return 0;
}
