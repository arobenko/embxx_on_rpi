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

#include "embxx/util/Assert.h"

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
        while (true) {;}
    }

private:
    Led& led_;
};

namespace log = embxx::util::log;
template <typename TLog, typename TTimer>
void performLog(TLog& log, TTimer& timer, std::size_t& counter)
{
    ++counter;

    SLOG(log, log::Info,
        "Logging output: counter = " <<
        embxx::io::dec << counter <<
        " (0x" << embxx::io::hex << counter << ")");

    // Perform next logging after a timeout
    static const std::size_t LoggingWaitPeriod = 1000; // 1 sec
    timer.asyncWait(
        LoggingWaitPeriod,
        std::bind(
            &performLog<TLog, TTimer>,
            std::ref(log),
            std::ref(timer),
            std::ref(counter)));
}

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();
    auto& log = system.log();

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& uart = system.uart();
    uart.configBaud(115200);
    uart.setWriteEnabled(true);

    auto timer = system.timerMgr().allocTimer();
    GASSERT(timer.isValid());
    std::size_t counter = 0;
    performLog(log, timer, counter);

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
