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
        device::interrupt::disable();
        while (true) {;}
    }

private:
    Led& led_;
};

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

    auto& spi = system.spi();
    spi.setFillChar(0xff);

    std::size_t cmdIdx = 0;

    spi.setCanWriteHandler(
        [&cmdIdx, &spi]()
        {
            while (spi.canWrite(embxx::device::context::Interrupt())) {
                spi.write(Cmd0[cmdIdx], embxx::device::context::Interrupt());
                ++cmdIdx;
            }
        });

    spi.setWriteCompleteHandler(
        [&log, &led](const embxx::error::ErrorStatus& es)
        {
            GASSERT(!es);
            SLOG(log, embxx::util::log::Info, "Command write complete!");
        });


    spi.startWrite(
        System::SpiDevIdx,
        Cmd0Size,
        embxx::device::context::EventLoop());

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
