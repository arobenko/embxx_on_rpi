//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#include "System.h"

#include <functional>

#include "embxx/util/Assert.h"

namespace
{

class LedOnAssert : public embxx::util::Assert
{
public:
    LedOnAssert(device::Led& led)
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
    device::Led& led_;
};

void writeChar(System::UartSocket& uartSocket, System::Uart::CharType& ch);

void readChar(System::UartSocket& uartSocket, System::Uart::CharType& ch)
{
    uartSocket.asyncRead(&ch, 1,
        [&uartSocket, &ch](embxx::driver::ErrorStatus status, std::size_t bytesRead)
        {
            GASSERT(status == embxx::driver::ErrorStatus::Success);
            GASSERT(bytesRead == 1);
            writeChar(uartSocket, ch);
        });
}

void writeChar(System::UartSocket& uartSocket, System::Uart::CharType& ch)
{
    uartSocket.asyncWrite(&ch, 1,
        [&uartSocket, &ch](embxx::driver::ErrorStatus status, std::size_t bytesWritten)
        {
            GASSERT(status == embxx::driver::ErrorStatus::Success);
            GASSERT(bytesWritten == 1);
            readChar(uartSocket, ch);
        });
}

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();
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
