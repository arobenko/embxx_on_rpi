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
        device::interrupt::disable();
        while (true) {;}
    }

private:
    Led& led_;
};

void writeChar(System::UartSocket& uartSocket, System::Uart::CharType& ch);

void readChar(System::UartSocket& uartSocket, System::Uart::CharType& ch)
{
    uartSocket.asyncRead(&ch, 1,
        [&uartSocket, &ch](const embxx::error::ErrorStatus& es, std::size_t bytesRead)
        {
            GASSERT(!es);
            GASSERT(bytesRead == 1);
            static_cast<void>(es);
            static_cast<void>(bytesRead);
            writeChar(uartSocket, ch);
        });
}

void writeChar(System::UartSocket& uartSocket, System::Uart::CharType& ch)
{
    uartSocket.asyncWrite(&ch, 1,
        [&uartSocket, &ch](const embxx::error::ErrorStatus& es, std::size_t bytesWritten)
        {
            GASSERT(status == embxx::driver::ErrorStatus::Success);
            GASSERT(bytesWritten == 1);
            static_cast<void>(es);
            static_cast<void>(bytesWritten);
            readChar(uartSocket, ch);
        });
}

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& uart = system.uart();
    uart.configBaud(115200);
    uart.setReadEnabled(true);
    uart.setWriteEnabled(true);

    auto& uartSocket = system.uartSocket();
    System::Uart::CharType ch = 0;
    readChar(uartSocket, ch);

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
