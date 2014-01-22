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

bool verifySeqCorrect(
    const System::I2C::CharType* buf,
    std::size_t bufSize)
{
    int step = 1;
    if (buf[0] != static_cast<System::I2C::CharType>(0)) {
        step = -1;
    }

    auto expValue = buf[0] + step;
    for (std::size_t idx = 1U; idx < bufSize; ++idx) {
        if (buf[idx] != expValue) {
            return false;
        }
        expValue += step;
    }

    return true;
}

void readFunc(
    System::Eeprom& eeprom,
    System::Eeprom::EepromAddressType address,
    System::I2C::CharType* buf,
    std::size_t bufSize,
    System::Eeprom::EepromAddressType maxAddress);


void writeFunc(
    System::Eeprom& eeprom,
    System::Eeprom::EepromAddressType address,
    System::I2C::CharType* buf,
    std::size_t bufSize,
    System::Eeprom::EepromAddressType maxAddress)
{
    if (maxAddress <= address) {
        readFunc(eeprom, 0, buf, bufSize, maxAddress);
        return;
    }

    auto bufIter = &buf[0];
    embxx::io::writeBig(address, bufIter);
    std::size_t writeCount = std::min(bufSize, std::size_t(maxAddress - address) + sizeof(address));

    eeprom.asyncWrite(buf, writeCount,
        [&eeprom, address, buf, bufSize, maxAddress, writeCount](const embxx::error::ErrorStatus& err, std::size_t bytesTransferred)
        {
            auto& log = System::instance().log();

            if (err) {
                SLOG(log, embxx::util::log::Error,
                    "W (0x" << embxx::io::hex << embxx::io::width(0) <<
                    eeprom.getDeviceId() << ") : Failed with error " <<
                    static_cast<int>(err.code()));
                return;
            }

            GASSERT(bytesTransferred == writeCount);
            static_cast<void>(bytesTransferred);
            auto writtenDataCount = writeCount - sizeof(address);
            SLOG(log, embxx::util::log::Info,
                "W (0x" << embxx::io::hex << embxx::io::width(0) <<
                eeprom.getDeviceId() << ") : [0x" <<
                embxx::io::fill('0') << embxx::io::width(sizeof(address) * 2) <<
                address << " - 0x" << address + (writtenDataCount - 1) <<
                "] : {0x" << embxx::io::width(sizeof(System::I2C::CharType) * 2) <<
                static_cast<unsigned>(buf[sizeof(address)]) << " .. 0x" <<
                static_cast<unsigned>(buf[writeCount - 1]) << "} " <<
                embxx::io::width(0) << embxx::io::dec << writtenDataCount << " bytes");

            writeFunc(eeprom, address + writtenDataCount, buf, bufSize, maxAddress);
        });
}

void readFunc(
    System::Eeprom& eeprom,
    System::Eeprom::EepromAddressType address,
    System::I2C::CharType* buf,
    std::size_t bufSize,
    System::Eeprom::EepromAddressType maxAddress)
{
    if (maxAddress <= address) {
        writeFunc(eeprom, 0, buf, bufSize, maxAddress);
        return;
    }

    auto bufIter = &buf[0];
    embxx::io::writeBig(address, bufIter);

    eeprom.asyncWrite(buf, sizeof(address),
        [&eeprom, address, buf, bufSize, maxAddress](const embxx::error::ErrorStatus& err, std::size_t bytesTransferred)
        {
            auto& log = System::instance().log();
            if (err) {
                SLOG(log, embxx::util::log::Error,
                    "R (0x" << embxx::io::hex << embxx::io::width(0) <<
                    eeprom.getDeviceId() << ") : Failed to set address with error " <<
                    static_cast<int>(err.code()));
                return;
            }

            static_cast<void>(err);
            GASSERT(bytesTransferred == sizeof(address));
            std::size_t readCount = std::min(bufSize - sizeof(address), std::size_t(maxAddress - address));

            eeprom.asyncRead(&buf[sizeof(address)], readCount,
                [&eeprom, address, buf, bufSize, maxAddress, readCount](const embxx::error::ErrorStatus& err, std::size_t bytesTransferred)
                {
                    auto& log = System::instance().log();

                    if (err) {
                        SLOG(log, embxx::util::log::Error,
                            "R (0x" << embxx::io::hex << embxx::io::width(0) <<
                            eeprom.getDeviceId() << ") : Failed to read with error " <<
                            static_cast<int>(err.code()));
                        return;
                    }

                    static_cast<void>(err);
                    GASSERT(bytesTransferred == readCount);

                    auto readBuf = &buf[sizeof(address)];
                    SLOG(log, embxx::util::log::Info,
                        "R (0x" << embxx::io::hex << embxx::io::width(0) <<
                        eeprom.getDeviceId() << ") : [0x" <<
                        embxx::io::fill('0') << embxx::io::width(sizeof(address) * 2) <<
                        address << " - 0x" << address + (readCount - 1) <<
                        "] : {0x" << embxx::io::width(sizeof(System::I2C::CharType) * 2) <<
                        static_cast<unsigned>(readBuf[0]) << " .. 0x" <<
                        static_cast<unsigned>(readBuf[readCount - 1]) << "} " <<
                        embxx::io::width(0) << embxx::io::dec << readCount << " bytes");

                    if (!verifySeqCorrect(&readBuf[0], readCount)) {
                        SLOG(log, embxx::util::log::Error,
                            "Read mismatch for 0x" << eeprom.getDeviceId() << "!!!");
                        return;
                    }

                    readFunc(eeprom, address + readCount, buf, bufSize, maxAddress);
                });
        });
}

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();

    // Led on on assertion failure.
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& log = system.log();
    SLOG(log, embxx::util::log::Info, "Starting Write...");

    static const std::size_t ChunkSize = 128; // Maximum supported by eeprom
    static const std::size_t BufSize = ChunkSize + sizeof(System::Eeprom::EepromAddressType);
    typedef std::array<System::Eeprom::CharType, BufSize> DataBuf;

    DataBuf data1;
    for (auto i = 0U; i < ChunkSize; ++i) {
        auto idx = i + sizeof(System::Eeprom::EepromAddressType);
        data1[idx] = static_cast<System::Eeprom::CharType>(i);
    }

    DataBuf data2;
    for (auto i = 0U; i < ChunkSize; ++i) {
        auto idx = i + sizeof(System::Eeprom::EepromAddressType);
        data2[idx] = static_cast<System::Eeprom::CharType>(ChunkSize - (i + 1));
    }

    static const System::Eeprom::AttemtsCountType EepromAttemptCount = 20;
    auto& eeprom1 = system.eeprom1();
    eeprom1.setAttemptsLimit(EepromAttemptCount);
    auto& eeprom2 = system.eeprom2();
    eeprom2.setAttemptsLimit(EepromAttemptCount);

    static const System::Eeprom::EepromAddressType MaxAddress = 4 * 1024; // 4KB

    writeFunc(eeprom1, 0, &data1[0], BufSize, MaxAddress);
    writeFunc(eeprom2, 0, &data2[0], BufSize, MaxAddress);

    device::interrupt::enable();
    auto& el = system.eventLoop();
    el.run();

    GASSERT(0); // Mustn't exit
	return 0;
}
