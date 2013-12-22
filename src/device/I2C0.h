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


#pragma once


#include <cstdint>
#include <algorithm>
#include <limits>

#include "embxx/util/Assert.h"
#include "embxx/util/StaticFunction.h"
#include "embxx/error/ErrorStatus.h"

#include "Function.h"

namespace device
{

template <typename TInterruptMgr,
          typename TCanReadHandler = embxx::util::StaticFunction<void ()>,
          typename TCanWriteHandler = embxx::util::StaticFunction<void ()>,
          typename TReadCompleteHandler = embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&)>,
          typename TWriteCompleteHandler = embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&)> >
class I2C0
{
public:

    typedef std::uint8_t CharType;
    typedef std::uint8_t AddressType; // Currently only 7 bit addresses are supported
    typedef std::uint16_t LengthType;

    typedef TInterruptMgr InterruptMgr;
    typedef TCanReadHandler CanReadHandler;
    typedef TCanWriteHandler CanWriteHandler;
    typedef TReadCompleteHandler ReadCompleteHandler;
    typedef TWriteCompleteHandler WriteCompleteHandler;
    typedef std::uint32_t EntryType;

    typedef typename InterruptMgr::IrqId IrqId;

    I2C0(InterruptMgr& interruptMgr, Function& funcDev);

    static EntryType getDivider();
    static void setDivider(EntryType value);

    template <typename TFunc>
    void setCanReadHandler(TFunc&& func);

    template <typename TFunc>
    void setCanWriteHandler(TFunc&& func);

    template <typename TFunc>
    void setReadCompleteHandler(TFunc&& func);

    template <typename TFunc>
    void setWriteCompleteHandler(TFunc&& func);


    void startRead(AddressType address, std::size_t length);
    bool cancelRead();
    void startWrite(AddressType address, std::size_t length);
    bool cancelWrite();

    bool cancelReadInterruptCtx();
    bool canRead();
    bool canWrite();
    CharType read();
    void write(CharType value);


private:
    enum class OpType {
        Idle,
        Read,
        Write
    };

    static void setReadInterruptEnabled(bool enabled);
    static void setWriteInterruptEnabled(bool enabled);
    static void setDoneInterruptEnabled(bool enabled);
    static void setReadTransfer(bool readTransfer);
    static void startTransfer();
    static void clearFifo();
    void interruptHandler();
    void completeTransfer(const embxx::error::ErrorStatus& status);
    void setAddrAndLen(AddressType address, LengthType length);

    CanReadHandler canReadHandler_;
    CanWriteHandler canWriteHandler_;
    ReadCompleteHandler readCompleteHandler_;
    WriteCompleteHandler writeCompleteHandler_;
    OpType op_;
    std::size_t remainingLen_;

    typedef Function::PinIdxType PinIdxType;
    typedef Function::FuncSel FuncSel;

    static constexpr EntryType genMask(std::size_t pos, std::size_t len = 1)
    {
        return ((static_cast<EntryType>(1) << len) - 1) << pos;
    }

    static const PinIdxType LineSDA0 = 0;
    static const PinIdxType LineSCL0 = 1;

    static const FuncSel AltFuncSDA0 = FuncSel::Alt0;
    static const FuncSel AltFuncSCL0 = FuncSel::Alt0;

    static constexpr auto pBSC0_C =
        reinterpret_cast<volatile EntryType*>(0x20205000);
    static const std::size_t ReadTransferPos = 0;
    static const std::size_t ClearFifoPos = 4;
    static const std::size_t ClearFifoLen = 2;
    static const std::size_t StartTransferPos = 7;
    static const std::size_t InterruptOnDonePos = 8;
    static const std::size_t InterruptOnTxPos = 9;
    static const std::size_t InterruptOnRxPos = 10;
    static const std::size_t I2CEnablePos = 15;
    static const EntryType pBSC0_C_UsedBits =
        genMask(ReadTransferPos) |
        genMask(ClearFifoPos, ClearFifoLen) |
        genMask(StartTransferPos) |
        genMask(InterruptOnDonePos) |
        genMask(InterruptOnTxPos) |
        genMask(InterruptOnRxPos) |
        genMask(I2CEnablePos);

    static constexpr auto pBSC0_S =
        reinterpret_cast<volatile EntryType*>(0x20205004);
    static const std::size_t TransferActivePos = 0;
    static const std::size_t TransferDonePos = 1;
    static const std::size_t FifoNeedsWritingPos = 2;
    static const std::size_t FifoNeedsReadingPos = 3;
    static const std::size_t FifoCanAcceptDataPos = 4;
    static const std::size_t FifoContainsDataPos = 5;
    static const std::size_t TxFifoEmptyPos = 6;
    static const std::size_t RxFifoFullPos = 7;
    static const std::size_t AckErrorPos = 8;
    static const std::size_t ClockStretchTimeoutPos = 9;
    static const EntryType pBSC0_S_UsedBits =
        genMask(TransferActivePos) |
        genMask(TransferDonePos) |
        genMask(FifoNeedsWritingPos) |
        genMask(FifoNeedsReadingPos) |
        genMask(FifoCanAcceptDataPos) |
        genMask(FifoContainsDataPos) |
        genMask(TxFifoEmptyPos) |
        genMask(RxFifoFullPos) |
        genMask(AckErrorPos) |
        genMask(ClockStretchTimeoutPos);

    static const EntryType pBSC0_S_WritableBits =
        genMask(TransferDonePos) |
        genMask(AckErrorPos) |
        genMask(ClockStretchTimeoutPos);


    static constexpr auto pBSC0_DLEN =
        reinterpret_cast<volatile EntryType*>(0x20205008);
    static const std::size_t DataLengthPos = 0;
    static const std::size_t DataLengthLen = 16;
    static const EntryType pBSC0_DLEN_UsedBits =
        genMask(DataLengthPos, DataLengthLen);

    static constexpr auto pBSC0_A =
        reinterpret_cast<volatile EntryType*>(0x2020500C);
    static const std::size_t SlaveAddressPos = 0;
    static const std::size_t SlaveAddressLen = 7;
    static const EntryType pBSC0_A_UsedBits =
        genMask(SlaveAddressPos, SlaveAddressLen);

    static constexpr auto pBSC0_FIFO =
        reinterpret_cast<volatile EntryType*>(0x20205010);
    static const std::size_t DataPos = 0;
    static const std::size_t DataLen = 8;
    static const std::size_t pBSC0_FIFO_UsedBits =
        genMask(DataPos, DataLen);

    static constexpr auto pBSC0_DIV =
        reinterpret_cast<volatile EntryType*>(0x20205014);
    static const std::size_t ClockDividerPos = 0;
    static const std::size_t ClockDividerLen = 16;

    static void setBits(
        volatile EntryType* entry,
        EntryType existingValueMask,
        bool value,
        std::size_t pos,
        std::size_t len = 1)
    {
        auto mask = genMask(pos, len);
        EntryType entryValue = *entry & existingValueMask;
        if (value) {
            entryValue |= mask;
        }
        else {
            entryValue &= (~mask);
        }
        *entry = entryValue;
    }
};

// Implementation
template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
I2C0(
    InterruptMgr& interruptMgr,
    Function& funcDev)
    : op_(OpType::Idle),
      remainingLen_(0)
{
    funcDev.configure(LineSDA0, AltFuncSDA0);
    funcDev.configure(LineSCL0, AltFuncSCL0);

    interruptMgr.registerHandler(
        IrqId::IrqId_I2C,
        std::bind(&I2C0::interruptHandler, this));

    *pBSC0_C = genMask(I2CEnablePos);
    clearFifo();

    interruptMgr.enableInterrupt(IrqId::IrqId_I2C);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
typename I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::EntryType
I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
getDivider()
{
    return *pBSC0_DIV & genMask(ClockDividerPos, ClockDividerLen);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setDivider(EntryType value)
{
    *pBSC0_DIV = value & genMask(ClockDividerPos, ClockDividerLen);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setCanReadHandler(
    TFunc&& func)
{
    canReadHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setCanWriteHandler(
    TFunc&& func)
{
    canWriteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setReadCompleteHandler(
    TFunc&& func)
{
    readCompleteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setWriteCompleteHandler(
    TFunc&& func)
{
    writeCompleteHandler_ = std::forward<TFunc>(func);
}


template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
startRead(
    AddressType address,
    std::size_t length)
{
    GASSERT(op_ == OpType::Idle);
    GASSERT(remainingLen_ == 0);
    GASSERT(length <= genMask(DataLengthPos, DataLengthLen));
    op_ = OpType::Read;
    setAddrAndLen(address, static_cast<LengthType>(length));
    setReadTransfer(true);
    setReadInterruptEnabled(true);
    startTransfer();
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
bool I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
cancelRead()
{
    return cancelReadInterruptCtx(); // The functionality is the same
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
startWrite(
    AddressType address,
    std::size_t length)
{
    GASSERT(op_ == OpType::Idle);
    GASSERT(remainingLen_ == 0);
    GASSERT(length <= genMask(DataLengthPos, DataLengthLen));
    op_ = OpType::Write;
    setAddrAndLen(address, length);
    setReadTransfer(false);
    setWriteInterruptEnabled(true);
    startTransfer();
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
bool I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
cancelWrite()
{
    GASSERT(op_ == OpType::Write);
    setWriteInterruptEnabled(false);
    setDoneInterruptEnabled(false);
    clearFifo();
    bool result = (0 < remainingLen_);
    remainingLen_ = 0;
    op_ = OpType::Idle;
    return result;
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
bool I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
cancelReadInterruptCtx()
{
    GASSERT(op_ == OpType::Read);
    setReadInterruptEnabled(false);
    setDoneInterruptEnabled(false);
    clearFifo();
    bool result = (0 < remainingLen_);
    remainingLen_ = 0;
    op_ = OpType::Idle;
    return result;
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
bool I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
canRead()
{
    return ((*pBSC0_S & genMask(FifoContainsDataPos)) != 0) &&
           (0 < remainingLen_);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
bool I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
canWrite()
{
    return ((*pBSC0_S & genMask(FifoCanAcceptDataPos)) != 0) &&
           (0 < remainingLen_);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
typename I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::CharType
I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
read()
{
    GASSERT(canRead());
    --remainingLen_;
    return static_cast<CharType>(
        *pBSC0_FIFO & genMask(DataPos, DataLen));
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
write(
    CharType value)
{
    GASSERT(canWrite());
    --remainingLen_;
    *pBSC0_FIFO =
        static_cast<EntryType>(value) & genMask(DataPos, DataLen);
}


template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setReadInterruptEnabled(
    bool enabled)
{
    setBits(pBSC0_C, pBSC0_C_UsedBits, enabled, InterruptOnRxPos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setWriteInterruptEnabled(
    bool enabled)
{
    setBits(pBSC0_C, pBSC0_C_UsedBits, enabled, InterruptOnTxPos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setDoneInterruptEnabled(
    bool enabled)
{
    setBits(pBSC0_C, pBSC0_C_UsedBits, enabled, InterruptOnDonePos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setReadTransfer(
    bool readTransfer)
{
    setBits(pBSC0_C, pBSC0_C_UsedBits, readTransfer, ReadTransferPos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
startTransfer()
{
    setDoneInterruptEnabled(true);
    setBits(pBSC0_C, pBSC0_C_UsedBits, true, StartTransferPos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
clearFifo()
{
    setBits(pBSC0_C, pBSC0_C_UsedBits, true, ClearFifoPos, ClearFifoLen);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
interruptHandler()
{
    EntryType status = *pBSC0_S;
    *pBSC0_S = status & pBSC0_S_WritableBits; // clear some bits

    static const auto errorMask =
        genMask(ClockStretchTimeoutPos) |
        genMask(AckErrorPos);

    if (((status & errorMask) != 0) ||
        (op_ != OpType::Idle)) {
        completeTransfer(embxx::error::ErrorCode::HwProtocolError);
        return;
    }

    if ((status & genMask(TransferDonePos)) &&
        ((*pBSC0_C & genMask(InterruptOnDonePos)) != 0) &&
        (op_ != OpType::Idle)) {

        if (0 < remainingLen_) {
            // Done when not all the data is read/written
            completeTransfer(embxx::error::ErrorCode::HwProtocolError);
            return;
        }

        completeTransfer(embxx::error::ErrorCode::Success);
        op_ = OpType::Idle;
        return;
    }

    if (op_ == OpType::Idle) {
        // should not happen, but to increase robustness...
        setReadInterruptEnabled(false);
        setWriteInterruptEnabled(false);
        setDoneInterruptEnabled(false);
        clearFifo();
        return;
    }

    if (((status & genMask(FifoNeedsReadingPos)) != 0) &&
        ((*pBSC0_C & genMask(InterruptOnRxPos)) != 0)) {
        GASSERT(canReadHandler_);
        canReadHandler_();

        if (remainingLen_ == 0) {
            setReadInterruptEnabled(false);
        }
        return;
    }

    if (((status & genMask(FifoNeedsWritingPos)) != 0) &&
        ((*pBSC0_C & genMask(InterruptOnTxPos)) != 0)) {
        GASSERT(canWriteHandler_);
        canWriteHandler_();

        if (remainingLen_ == 0) {
            setWriteInterruptEnabled(false);
        }
        return;
    }

    // Should not be here, spurious interrupt
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
completeTransfer(
    const embxx::error::ErrorStatus& status)
{
    setReadInterruptEnabled(false);
    setWriteInterruptEnabled(false);
    setDoneInterruptEnabled(false);

    if (op_ == OpType::Read) {
        GASSERT(readCompleteHandler_);
        readCompleteHandler_(status);
    }
    else  {
        GASSERT(op_ == OpType::Write);
        GASSERT(writeCompleteHandler_);
        writeCompleteHandler_(status);
    }
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler,
          typename TReadCompleteHandler,
          typename TWriteCompleteHandler>
void I2C0<TInterruptMgr, TCanReadHandler, TCanWriteHandler, TReadCompleteHandler, TWriteCompleteHandler>::
setAddrAndLen(
    AddressType address,
    LengthType length)
{
    static_assert(
        std::numeric_limits<LengthType>::max() <= genMask(DataLengthPos, DataLengthLen),
        "Length type assumption is incorrect");

    GASSERT(address <= 0x7f);

    *pBSC0_A = address & genMask(SlaveAddressPos, SlaveAddressLen);
    *pBSC0_DLEN = static_cast<EntryType>(length);
    remainingLen_ = length;
}

}  // namespace device



