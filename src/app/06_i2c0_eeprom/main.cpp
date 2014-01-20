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
#include "embxx/util/StreamLogger.h"

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

    led.on();
    for (volatile auto i = 0; i < 5000000; ++i) {};
    led.off();
    for (volatile auto i = 0; i < 5000000; ++i) {};

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& log = system.log();
    SLOG(log, embxx::util::log::Info, "Hello");


    static const System::I2C::CharType Buf[] =
    {
        0x04, 0x00, 0xde, 0xad,
        0xde, 0xad, 0xde, 0xad,
        0xde, 0xad, 0xde, 0xad,
        0xde, 0xad, 0xde, 0xad,
        0xde, 0xad, 0xde, 0xad
    };

    static const std::size_t BufSize = sizeof(Buf)/sizeof(Buf[0]);

    System::I2C::CharType readBuf[BufSize - 2] = {0};

    auto& eeprom = system.eeprom();
    eeprom.asyncWrite(&Buf[0], BufSize,
        [&](const embxx::error::ErrorStatus& err, std::size_t bytesTransferred)
        {
            SLOG(log, embxx::util::log::Info, "Write Callback is called: " << bytesTransferred);
            GASSERT(!err);
            static_cast<void>(err);
            static_cast<void>(bytesTransferred);

            eeprom.asyncWrite(&Buf[0], 2,
                [&](const embxx::error::ErrorStatus& err, std::size_t bytesTransferred)
                {
                    SLOG(log, embxx::util::log::Info, "Address Write Callback is called: " << (int)err.code() << "; " << bytesTransferred);

                    GASSERT(!err);
                    static_cast<void>(err);
                    static_cast<void>(bytesTransferred);

                    eeprom.asyncRead(&readBuf[0], BufSize - 2,
                        [&](const embxx::error::ErrorStatus& err, std::size_t bytesTransferred)
                        {
                            SLOG(log, embxx::util::log::Info, "Read Callback is called: " << (int)err.code() << "; " << bytesTransferred);
                            GASSERT(!err);
                            static_cast<void>(err);
                            static_cast<void>(bytesTransferred);
                            GASSERT(std::equal(&Buf[2], &Buf[0] + BufSize, &readBuf[0]));
                        });
                });
        });

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
