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
        SLOG(log, embxx::util::log::Info, "Assertion `" << expr << "` in " << function << " at " << file << ":" << line);
    }

private:
    Led& led_;
};

void processSpiInBuf(
    System::SpiInStreamBuf& spiInBuf,
    System::Log& log,
    std::size_t remainingCount)
{
    if (remainingCount == 0) {
        spiInBuf.stop();
        return;
    }

    spiInBuf.asyncWaitDataAvailable(
        1,
        [&spiInBuf, &log, remainingCount](const embxx::error::ErrorStatus& es)
        {
            if (!es) {
                auto iter = spiInBuf.begin();
                auto byte = embxx::io::readBig<std::uint8_t>(iter);
                SLOG(log, embxx::util::log::Info, "Byte=0x" << embxx::io::hex << (unsigned)byte);
                spiInBuf.consume(1U);
            }
            else {
                SLOG(log, embxx::util::log::Error, "Error " << embxx::io::dec << (unsigned)es.code());
            }
            processSpiInBuf(spiInBuf, log, remainingCount - 1);
        });
}

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    static const std::uint8_t Cmd0[] = {
        0x40, 0x0, 0x0, 0x0, 0x0, 0x95
    };

    static const std::size_t Cmd0Size = std::extent<decltype(Cmd0)>::value;

    auto& log = system.log();
    SLOG(log, embxx::util::log::Info, "Starting Write... (" << Cmd0Size << ")");

    led.on();
    for (volatile int i = 0; i < 5000000; ++i) {}
    led.off();

    // TODO: check what's going on with spi cancellation.

    auto& spi = system.spi();
    spi.device().device().device().setFillChar(0xff);

    spi.asyncWrite(
        &Cmd0[0],
        Cmd0Size,
        [&log, &spi](const embxx::error::ErrorStatus& es, std::size_t bytesWritten)
        {
            SLOG(log, embxx::util::log::Info, "Write 1 complete!");
            GASSERT(!es);
            GASSERT(bytesWritten == Cmd0Size);

            spi.asyncWrite(
                &Cmd0[0],
                Cmd0Size,
                [&log, &spi](const embxx::error::ErrorStatus& es, std::size_t bytesWritten)
                {
                    SLOG(log, embxx::util::log::Info, "Write 2 complete!");
                    GASSERT(!es);
                    GASSERT(bytesWritten == Cmd0Size);
                });

        });

    auto& spiInBuf = system.spiInBuf();
    spiInBuf.start();
    processSpiInBuf(spiInBuf, log, 1024);

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
