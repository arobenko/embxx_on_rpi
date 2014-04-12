//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
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


#include <cstdint>
#include <algorithm>
#include <limits>

#include "embxx/util/Assert.h"
#include "embxx/util/StaticFunction.h"
#include "embxx/util/ScopeGuard.h"
#include "embxx/error/ErrorStatus.h"
#include "embxx/device/context.h"
#include "embxx/device/op_category.h"

#include "Function.h"

namespace device
{

template <typename TInterruptMgr,
          typename TCanDoHandler = embxx::util::StaticFunction<void ()>,
          typename TOpCompleteHandler = embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&)> >
class Spi0
{
public:

    typedef std::uint8_t CharType;
    typedef unsigned DeviceIdType;
    typedef embxx::device::op_category::ParallelReadWrite OpCategory;

    static const DeviceIdType SupportedDeviceIdsCount = 3U;

    typedef TInterruptMgr InterruptMgr;
    typedef TCanDoHandler CanReadHandler;
    typedef TCanDoHandler CanWriteHandler;
    typedef TOpCompleteHandler ReadCompleteHandler;
    typedef TOpCompleteHandler WriteCompleteHandler;
    typedef std::uint32_t EntryType;

    typedef embxx::device::context::EventLoop EventLoopContext;
    typedef embxx::device::context::Interrupt InterruptContext;

    enum Mode
    {
        Mode0,
        Mode1,
        Mode2,
        Mode3,
        NumOfModes
    };

    Spi0(InterruptMgr& interruptMgr, Function& funcDev, Mode mode = Mode0);

    static unsigned getFreq(unsigned sysFreq);
    static void setFreq(unsigned sysFreq, unsigned bufFreq);

    Mode getMode() const;
    void setMode(Mode mode);

    CharType getFillChar() const;
    void setFillChar(CharType ch);

    template <typename TFunc>
    void setCanReadHandler(TFunc&& func);

    template <typename TFunc>
    void setCanWriteHandler(TFunc&& func);

    template <typename TFunc>
    void setReadCompleteHandler(TFunc&& func);

    template <typename TFunc>
    void setWriteCompleteHandler(TFunc&& func);

    void startRead(
        DeviceIdType id,
        std::size_t length,
        EventLoopContext context);

    void startRead(
        DeviceIdType id,
        std::size_t length,
        InterruptContext context);

    bool cancelRead(EventLoopContext context);
    bool cancelRead(InterruptContext context);

    void startWrite(
        DeviceIdType id,
        std::size_t length,
        EventLoopContext context);

    void startWrite(
        DeviceIdType id,
        std::size_t length,
        InterruptContext context);

    bool cancelWrite(EventLoopContext context);
    bool cancelWrite(InterruptContext context);

    bool suspend(EventLoopContext context);
    void resume(EventLoopContext context);

    bool canRead(InterruptContext context);
    bool canWrite(InterruptContext context);
    CharType read(InterruptContext context);
    void write(CharType value, InterruptContext context);


private:
    typedef typename InterruptMgr::IrqId IrqId;

    void selectChip(DeviceIdType id);
    DeviceIdType getChip() const;
    void startReadInternal(DeviceIdType id, std::size_t length);
    void startWriteInternal(DeviceIdType id, std::size_t length);
    bool cancelReadInternal();
    bool cancelWriteInternal();
    void disableInterrupts();
    void enableInterrupts();
    void startTransfer(DeviceIdType id);
    void stopTransfer();
    void interruptHandler();
    void reportReadComplete(const embxx::error::ErrorStatus& es);
    void reportWriteComplete(const embxx::error::ErrorStatus& es);
    void readFromFifo(std::size_t maxCount);
    void writeToFifo(std::size_t maxCount);
    bool readOpInProgress() const;
    bool writeOpInProgress() const;

    InterruptMgr& interruptMgr_;
    CanReadHandler canReadHandler_;
    CanWriteHandler canWriteHandler_;
    ReadCompleteHandler readCompleteHandler_;
    WriteCompleteHandler writeCompleteHandler_;
    std::size_t remainingReadLen_;
    std::size_t remainingWriteLen_;
    std::size_t readFifoSize_;
    std::size_t writeFifoSize_;
    volatile EntryType csCache_;
    CharType fillChar_;

    typedef Function::PinIdxType PinIdxType;
    typedef Function::FuncSel FuncSel;

    static const PinIdxType LineCS0 = 8;
    static const PinIdxType LineCS1 = 7;
    static const PinIdxType LineMISO = 9;
    static const PinIdxType LineMOSI = 10;
    static const PinIdxType LineSCLK = 11;

    static const FuncSel AltFuncAll = FuncSel::Alt0;

    static constexpr EntryType genMask(std::size_t pos, std::size_t len = 1)
    {
        return ((static_cast<EntryType>(1) << len) - 1) << pos;
    }

    static constexpr auto pSPI0_CS =
        reinterpret_cast<volatile EntryType*>(0x20204000);
    static const std::size_t SPI0_CS_CS_Pos = 0;
    static const std::size_t SPI0_CS_CS_Len = 2;
    static const std::size_t SPI0_CS_ModePos = 2;
    static const std::size_t SPI0_CS_ModeLen = 2;
    static const std::size_t SPI0_CS_CLEAR_Pos = 4;
    static const std::size_t SPI0_CS_CLEAR_Len = 2;
    static const std::size_t SPI0_CS_TA_Pos = 7;
    static const std::size_t SPI0_CS_INTD_Pos = 9;
    static const std::size_t SPI0_CS_INTR_Pos = 10;
    static const std::size_t SPI0_CS_DONE_Pos = 16;
    static const std::size_t SPI0_CS_RXD_Pos = 17;
    static const std::size_t SPI0_CS_TXD_Pos = 18;
    static const std::size_t SPI0_CS_RXR_Pos = 19;

    static const EntryType SPI0_CS_CS =
        genMask(SPI0_CS_CS_Pos, SPI0_CS_CS_Len);

    static const EntryType SPI0_CS_CLEAR =
        genMask(SPI0_CS_CLEAR_Pos, SPI0_CS_CLEAR_Len);

    static const EntryType SPI0_CS_TA = genMask(SPI0_CS_TA_Pos);
    static const EntryType SPI0_CS_INTD = genMask(SPI0_CS_INTD_Pos);
    static const EntryType SPI0_CS_INTR = genMask(SPI0_CS_INTR_Pos);
    static const EntryType SPI0_CS_DONE = genMask(SPI0_CS_DONE_Pos);
    static const EntryType SPI0_CS_RXD = genMask(SPI0_CS_RXD_Pos);
    static const EntryType SPI0_CS_TXD = genMask(SPI0_CS_TXD_Pos);
    static const EntryType SPI0_CS_RXR = genMask(SPI0_CS_RXR_Pos);

    static const EntryType TranfserActiveMask = SPI0_CS_TA | SPI0_CS_INTD | SPI0_CS_INTR;


    static const std::size_t pSPI0_CS_FifoNeedsReadStatusPos = 19;

    static constexpr auto pSPI0_FIFO =
        reinterpret_cast<volatile EntryType*>(0x20204004);


    static constexpr auto pSPI0_CLK =
        reinterpret_cast<volatile EntryType*>(0x20204008);

    static const std::size_t pSPI0_CLK_CDIV_Pos = 0;
    static const std::size_t pSPI0_CLK_CDIV_Len = 15;
};

// Implementation
template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::Spi0(
    InterruptMgr& interruptMgr,
    Function& funcDev,
    Mode mode)
    : interruptMgr_(interruptMgr),
      remainingReadLen_(0),
      remainingWriteLen_(0),
      readFifoSize_(0),
      writeFifoSize_(0),
      csCache_(0),
      fillChar_(0)
{
    funcDev.configure(LineCS0, AltFuncAll);
    funcDev.configure(LineCS1, AltFuncAll);
    funcDev.configure(LineMOSI, AltFuncAll);
    funcDev.configure(LineMISO, AltFuncAll);
    funcDev.configure(LineSCLK, AltFuncAll);

    interruptMgr.registerHandler(
        IrqId::IrqId_SPI,
        std::bind(&Spi0::interruptHandler, this));

    setMode(mode);

    *pSPI0_CS = csCache_ | SPI0_CS_CLEAR;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
unsigned Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::getFreq(
    unsigned sysFreq)
{
    static const unsigned ZeroDiv =
        genMask(pSPI0_CLK_CDIV_Pos, pSPI0_CLK_CDIV_Len) + 1;

    auto div = *pSPI0_CLK & genMask(pSPI0_CLK_CDIV_Pos, pSPI0_CLK_CDIV_Len);
    if (div == 0) {
        div = ZeroDiv;
    }

    return sysFreq / div;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::setFreq(
    unsigned sysFreq,
    unsigned busFreq)
{
    unsigned div = 0;

    do {
        if (sysFreq <= busFreq) {
            div = 2;
            break;
        }

        div = (sysFreq + busFreq - 1) / busFreq; // round up (sysFreq / busFreq);

        // round up till power of two
        if ((div & (div - 1)) != 0) {
            div <<= 1;

            while (true) {
                auto tmp = div & (div - 1);
                if (tmp == 0) {
                    break;
                }
                div = tmp;
            }
        }

        div = std::max(div, 2U);

        static const auto MaxDiv =
            genMask(pSPI0_CLK_CDIV_Pos, pSPI0_CLK_CDIV_Len) + 1;
        if (MaxDiv <= div) {
            div = 0;
        }

    } while (false);

    *pSPI0_CLK = div & genMask(pSPI0_CLK_CDIV_Pos, pSPI0_CLK_CDIV_Len);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::Mode
Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::getMode() const
{
    auto mode =
        csCache_ & genMask(SPI0_CS_ModePos, SPI0_CS_ModeLen) >> SPI0_CS_ModeLen;
    return static_cast<Mode>(mode);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::setMode(Mode mode)
{
    static const auto Mask = genMask(SPI0_CS_ModePos, SPI0_CS_ModeLen);
    csCache_&= ~Mask;
    csCache_ |= ((mode << SPI0_CS_ModeLen) & Mask);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::CharType
Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::getFillChar() const
{
    return fillChar_;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::setFillChar(
    CharType ch)
{
    fillChar_ = ch;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setCanReadHandler(
    TFunc&& func)
{
    canReadHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setCanWriteHandler(
    TFunc&& func)
{
    canWriteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setReadCompleteHandler(
    TFunc&& func)
{
    readCompleteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setWriteCompleteHandler(
    TFunc&& func)
{
    writeCompleteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startRead(
    DeviceIdType id,
    std::size_t length,
    EventLoopContext context)
{
    static_cast<void>(context);
    disableInterrupts();
    startReadInternal(id, length);
    enableInterrupts();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startRead(
    DeviceIdType id,
    std::size_t length,
    InterruptContext context)
{
    static_cast<void>(context);
    startReadInternal(id, length);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelRead(
    EventLoopContext context)
{
    static_cast<void>(context);
    disableInterrupts();
    bool result = cancelReadInternal();
    enableInterrupts();
    return result;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelRead(
    InterruptContext context)
{
    static_cast<void>(context);
    return cancelReadInternal();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startWrite(
    DeviceIdType id,
    std::size_t length,
    EventLoopContext context)
{
    static_cast<void>(context);
    disableInterrupts();
    startWriteInternal(id, length);
    enableInterrupts();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startWrite(
    DeviceIdType id,
    std::size_t length,
    InterruptContext context)
{
    static_cast<void>(context);
    startWriteInternal(id, length);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelWrite(
    EventLoopContext context)
{
    static_cast<void>(context);
    disableInterrupts();
    bool result = cancelWriteInternal();
    enableInterrupts();
    return result;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelWrite(
    InterruptContext context)
{
    static_cast<void>(context);
    return cancelWriteInternal();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::suspend(
    EventLoopContext context)
{
    disableInterrupts();
    return (readOpInProgress() || writeOpInProgress());
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::resume(
    EventLoopContext context)
{
    GASSERT(readOpInProgress() || writeOpInProgress());
    enableInterrupts();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::canRead(
    InterruptContext context)
{
    static_cast<void>(context);
    return ((0 < readFifoSize_) && ((*pSPI0_CS & SPI0_CS_RXD) != 0));
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::canWrite(
    InterruptContext context)
{
    static_cast<void>(context);
    return ((0 < writeFifoSize_) && ((*pSPI0_CS & SPI0_CS_TXD) != 0));
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::CharType
Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::read(
    InterruptContext context)
{
    static_cast<void>(context);
    GASSERT(canRead(context));
    --readFifoSize_;
    --remainingReadLen_;
    return static_cast<CharType>(*pSPI0_FIFO);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::write(
    CharType value,
    InterruptContext context)
{
    static_cast<void>(context);
    GASSERT(canWrite(context));
    --writeFifoSize_;
    --remainingWriteLen_;
    *pSPI0_FIFO = value;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::selectChip(
    DeviceIdType id)
{
    GASSERT(id < SupportedDeviceIdsCount);

    csCache_ &= ~SPI0_CS_CS;
    csCache_ |= (id << SPI0_CS_CS_Pos) & SPI0_CS_CS;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::DeviceIdType
Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::getChip() const
{
    return (csCache_ & SPI0_CS_CS) >> SPI0_CS_CS_Pos;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startReadInternal(
    DeviceIdType id,
    std::size_t length)
{
    GASSERT(remainingReadLen_ == 0);
    GASSERT(0 < length);
    remainingReadLen_ = length;
    if ((csCache_ & TranfserActiveMask) == 0) {
        startTransfer(id);
    }
    else {
        GASSERT(getChip() == id);
        GASSERT((csCache_ & TranfserActiveMask) == TranfserActiveMask);
    }
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startWriteInternal(
    DeviceIdType id,
    std::size_t length)
{
    GASSERT(remainingWriteLen_ == 0);
    GASSERT(0 < length);
    remainingWriteLen_ = length;

    if ((csCache_ & TranfserActiveMask) == 0) {
        startTransfer(id);
    }
    else {
        GASSERT(getChip() == id);
        GASSERT((csCache_ & TranfserActiveMask) == TranfserActiveMask);
    }
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelReadInternal()
{
    bool result = false;
    if (readOpInProgress()) {
        remainingReadLen_= 0;
        result = true;
    }

    return result;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelWriteInternal()
{
    bool result = false;
    if (writeOpInProgress()) {
        remainingWriteLen_= 0;
        result = true;
    }

    return result;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::disableInterrupts()
{
    interruptMgr_.disableInterrupt(IrqId::IrqId_SPI);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::enableInterrupts()
{
    interruptMgr_.enableInterrupt(IrqId::IrqId_SPI);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startTransfer(
    DeviceIdType id)
{
    GASSERT(writeOpInProgress() || readOpInProgress());
    GASSERT((csCache_ & TranfserActiveMask) == 0);
    selectChip(id);
    csCache_ |= TranfserActiveMask;
    *pSPI0_CS = csCache_;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::stopTransfer()
{
    csCache_ &= ~(TranfserActiveMask);
    *pSPI0_CS = csCache_;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
interruptHandler()
{
    static const std::size_t MaxFifoLen = 16U;
    static const std::size_t MidOpFifoLen = 12U;

    volatile auto csValue = *pSPI0_CS;

    auto readFunc =
        [this](bool reading, std::size_t limit)
        {
            readFromFifo(limit);
            if (reading && (!readOpInProgress())) {
                reportReadComplete(embxx::error::ErrorCode::Success);
            }
        };

    auto writeFunc =
        [this](bool writing, std::size_t limit)
        {
            writeToFifo(limit); // will write provided data + fill character if needed

            if (writing && (!writeOpInProgress())) {
                reportWriteComplete(embxx::error::ErrorCode::Success);
            }
        };

    if ((csValue & SPI0_CS_DONE) != 0) {

        if ((!readOpInProgress()) && (!writeOpInProgress())) {
            stopTransfer();
            return;
        }

        if (!writeOpInProgress()) {

            readFunc(true, MaxFifoLen);

            if (!readOpInProgress()) {
                return;
            }

            writeFunc(false, MaxFifoLen);
            return;
        }

        // First write
        readFunc(readOpInProgress(), MaxFifoLen);
        writeFunc(true, MaxFifoLen);
        return;
    }

    if ((csValue & SPI0_CS_RXR) != 0) {
        readFunc(readOpInProgress(), MidOpFifoLen);
        writeFunc(writeOpInProgress(), MidOpFifoLen);
    }
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::reportReadComplete(
    const embxx::error::ErrorStatus& es)
{
    GASSERT(readCompleteHandler_);
    remainingReadLen_ = 0;
    readCompleteHandler_(es);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::reportWriteComplete(
    const embxx::error::ErrorStatus& es)
{
    GASSERT(writeCompleteHandler_);
    remainingWriteLen_ = 0;
    writeCompleteHandler_(es);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::readFromFifo(
    std::size_t maxCount)
{
    if ((*pSPI0_CS & SPI0_CS_RXD) == 0) {
        return;
    }

    auto requestedRead = std::min(remainingReadLen_, maxCount);
    readFifoSize_ = requestedRead;

    if (0 < readFifoSize_) {
        GASSERT(canReadHandler_);
        canReadHandler_();
    }

    auto actuallyRead = requestedRead - readFifoSize_;
    auto remToRead = maxCount - actuallyRead;
    while ((0 < remToRead) &&
           ((*pSPI0_CS & SPI0_CS_RXD) != 0)) {
        auto ch = *pSPI0_FIFO;
        static_cast<void>(ch);
        --remToRead;
    }
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::writeToFifo(
    std::size_t maxCount)
{
    auto requestedWrite = std::min(remainingWriteLen_, maxCount);
    writeFifoSize_ = requestedWrite;

    if (0 < writeFifoSize_) {
        GASSERT(canWriteHandler_);
        canWriteHandler_();
    }

    auto actuallyWritten = requestedWrite - writeFifoSize_;
    auto remToWrite = maxCount - actuallyWritten;
    while ((0 < remToWrite) &&
           ((*pSPI0_CS & SPI0_CS_TXD) != 0)) {
        *pSPI0_FIFO = fillChar_;
        --remToWrite;
    }
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::readOpInProgress() const
{
    return (0 < remainingReadLen_);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Spi0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::writeOpInProgress() const
{
    return (0 < remainingWriteLen_);
}

}  // namespace device

