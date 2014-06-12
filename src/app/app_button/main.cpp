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
#include <chrono>

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
        device::interrupt::disable();
        while (true) {;}
    }

private:
    Led& led_;
};

template <typename TTimer>
void buttonPressed(System::EventLoop& el, TTimer& timer)
{
    static_cast<void>(timer);
    auto& system = System::instance();
    auto& led = system.led();
    auto& socket = system.uartSocket();

    static const char Str[] = "Button Pressed\r\n";
    static const std::size_t StrSize = sizeof(Str) - 1;
    socket.asyncWrite(Str, StrSize);

    timer.cancel();
    auto result = el.post(
        [&led]()
        {
            led.on();
        });
    GASSERT(result);
    static_cast<void>(result);

    static const auto WaitTime = std::chrono::seconds(1);
    timer.asyncWait(
        WaitTime,
        [&led](const embxx::error::ErrorStatus& es)
        {
            if (es == embxx::error::ErrorCode::Aborted) {
                return;
            }
            led.off();
        });
}

void buttonReleased()
{
    auto& system = System::instance();
    auto& socket = system.uartSocket();

    static const char Str[] = "Button Released\r\n";
    static const std::size_t StrSize = sizeof(Str) - 1;
    socket.asyncWrite(Str, StrSize);
}

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();
    auto& el = system.eventLoop();

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& uart = system.uart();
    uart.configBaud(9600);
    uart.setWriteEnabled(true);

    auto& timerMgr = system.timerMgr();
    auto timer = timerMgr.allocTimer();
    GASSERT(timer.isValid());

    auto& button = system.button();
    button.setPressedHandler(
        std::bind(
            &buttonPressed<decltype(timer)>,
            std::ref(el),
            std::ref(timer)));

    button.setReleasedHandler(&buttonReleased);

    device::interrupt::enable();

    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
