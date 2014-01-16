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

#include "embxx/util/Assert.h"
#include "embxx/util/StaticFunction.h"
#include "embxx/error/ErrorStatus.h"
#include "embxx/device/context.h"

#include "Function.h"

namespace device
{

template <typename TInterruptMgr,
          typename TCanDoHandler = embxx::util::StaticFunction<void ()>,
          typename TOpCompleteHandler = embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&)> >
class Uart1
{
public:

    typedef char CharType;

    typedef TInterruptMgr InterruptMgr;
    typedef TCanDoHandler CanReadHandler;
    typedef TCanDoHandler CanWriteHandler;
    typedef TOpCompleteHandler ReadCompleteHandler;
    typedef TOpCompleteHandler WriteCompleteHandler;

    typedef typename InterruptMgr::IrqId IrqId;

    typedef embxx::device::context::EventLoop EventLoopContext;
    typedef embxx::device::context::Interrupt InterruptContext;

    Uart1(InterruptMgr& interruptMgr, Function& funcDev, unsigned sysClock);

    static void setReadEnabled(bool enabled);
    static void setWriteEnabled(bool enabled);
    void configBaud(unsigned baud);

    template <typename TFunc>
    void setCanReadHandler(TFunc&& func);

    template <typename TFunc>
    void setCanWriteHandler(TFunc&& func);

    template <typename TFunc>
    void setReadCompleteHandler(TFunc&& func);

    template <typename TFunc>
    void setWriteCompleteHandler(TFunc&& func);

    void startRead(std::size_t length, EventLoopContext context);

    template <typename TContext>
    bool cancelRead(TContext context);

    void startWrite(std::size_t length, EventLoopContext context);
    bool cancelWrite(EventLoopContext context);

    bool canRead(InterruptContext context);
    bool canWrite(InterruptContext context);
    CharType read(InterruptContext context);
    void write(CharType value, InterruptContext context);

private:
    bool cancelReadInternal();
    static void setReadInterruptEnabled(bool enabled);
    static void setWriteInterruptEnabled(bool enabled);
    void interruptHandler();
    static bool isReadInterruptEnabled();
    static bool isWriteInterruptEnabled();

    unsigned sysClock_;
    CanReadHandler canReadHandler_;
    CanWriteHandler canWriteHandler_;
    ReadCompleteHandler readCompleteHandler_;
    WriteCompleteHandler writeCompleteHandler_;
    std::size_t remainingReadCount_;
    std::size_t remainingWriteCount_;

    typedef std::uint32_t EntryType;
    typedef Function::PinIdxType PinIdxType;
    typedef Function::FuncSel FuncSel;

    static constexpr EntryType genMask(std::size_t pos, std::size_t len = 1)
    {
        return ((static_cast<EntryType>(1) << len) - 1) << pos;
    }

    static const PinIdxType LineTXD1 = 14;
    static const PinIdxType LineRXD1 = 15;

    static const FuncSel AltFuncTXD1 = FuncSel::Alt5;
    static const FuncSel AltFuncRXD1 = FuncSel::Alt5;

    static constexpr auto pAUX_ENABLES =
        reinterpret_cast<volatile EntryType*>(0x20215004);
    static const std::size_t MiniUartEnablePos = 0;
    static const EntryType pAUX_ENABLES_UsedBits =
        genMask(MiniUartEnablePos);

    static constexpr auto pAUX_MU_IO_REG =
        reinterpret_cast<volatile EntryType*>(0x20215040);
    static const std::size_t IoDataPos = 0;
    static const std::size_t IoDataLen = 8;
    static const EntryType pAUX_MU_IO_REG_UsedBits =
        genMask(IoDataPos, IoDataLen);

    static constexpr auto pAUX_MU_IER_REG =
        reinterpret_cast<volatile EntryType*>(0x20215044);
    static const std::size_t EnableRxInterruptPos = 0; // datasheet is wrong, see 16550 spec
    static const std::size_t EnableTxInterruptPos = 1;// datasheet is wrong, see 16550 spec
    static const EntryType pAUX_MU_IER_REG_UsedBits =
        genMask(EnableTxInterruptPos) |
        genMask(EnableRxInterruptPos);


    static constexpr auto pAUX_MU_IIR_REG =
        reinterpret_cast<volatile EntryType*>(0x20215048);
    static const std::size_t FifoEnablePos = 6;
    static const std::size_t FifoEnableLen = 2;
    static const std::size_t TxInterruptPos = 1;
    static const std::size_t RxInterruptPos = 2;
    static const EntryType pAUX_MU_IIR_REG_UsedBits =
        genMask(FifoEnablePos, FifoEnableLen) |
        genMask(TxInterruptPos) |
        genMask(RxInterruptPos);

    static constexpr auto pAUX_MU_LCR_REG =
        reinterpret_cast<volatile EntryType*>(0x2021504C);
    static const std::size_t DataSizePos = 0;
    static const std::size_t DataSizeLen = 2;
    static const EntryType pAUX_MU_LCR_REG_UsedBits =
        genMask(DataSizePos, DataSizeLen);

    static constexpr auto pAUX_MU_LSR_REG =
        reinterpret_cast<volatile EntryType*>(0x20215054);
    static const std::size_t DataReadyPos = 0;
    static const std::size_t TransmitterEmptyPos = 5;


    static constexpr auto pAUX_MU_CNTL_REG =
        reinterpret_cast<volatile EntryType*>(0x20215060);
    static const std::size_t ReceiverEnablePos = 0;
    static const std::size_t TransmitterEnablePos = 1;
    static const EntryType pAUX_MU_CNTL_REG_UsedBits =
        genMask(ReceiverEnablePos) |
        genMask(TransmitterEnablePos);

    static constexpr auto pAUX_MU_BAUD_REG =
        reinterpret_cast<volatile EntryType*>(0x20215068);
    static const std::size_t BaudRatePos = 0;
    static const std::size_t BaudRateLen = 16;
    static const EntryType pAUX_MU_BAUD_REG_UsedBits =
        genMask(BaudRatePos, BaudRateLen);


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
          typename TCanDoHandler,
          typename TOpCompleteHandler>
Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
Uart1(
    InterruptMgr& interruptMgr,
    Function& funcDev,
    unsigned sysClock)
    : sysClock_(sysClock),
      remainingReadCount_(0),
      remainingWriteCount_(0)
{
    funcDev.configure(LineTXD1, AltFuncTXD1);
    funcDev.configure(LineRXD1, AltFuncRXD1);

    *pAUX_ENABLES = genMask(MiniUartEnablePos);

    setReadEnabled(false);
    setWriteEnabled(false);

    // Hardcoded to 8 bytes
    *pAUX_MU_LCR_REG = genMask(DataSizePos, DataSizeLen);

    setReadInterruptEnabled(false);
    setWriteInterruptEnabled(false);

    // Clear queues
    *pAUX_MU_IIR_REG =
        genMask(TxInterruptPos) |
        genMask(RxInterruptPos) |
        genMask(FifoEnablePos, FifoEnableLen);

    interruptMgr.registerHandler(
        IrqId::IrqId_AuxInt,
        std::bind(&Uart1::interruptHandler, this));
    interruptMgr.enableInterrupt(IrqId::IrqId_AuxInt);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setReadEnabled(
    bool enabled)
{
    setBits(pAUX_MU_CNTL_REG, pAUX_MU_CNTL_REG_UsedBits, enabled, ReceiverEnablePos);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setWriteEnabled(
    bool enabled)
{
    setBits(pAUX_MU_CNTL_REG, pAUX_MU_CNTL_REG_UsedBits, enabled, TransmitterEnablePos);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
configBaud(
    unsigned baud)
{
    auto regValue = ((sysClock_ / baud) / 8) - 1;
    GASSERT(regValue <= genMask(BaudRatePos, BaudRateLen));
    *pAUX_MU_BAUD_REG = static_cast<EntryType>(regValue);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setCanReadHandler(
    TFunc&& func)
{
    canReadHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setCanWriteHandler(
    TFunc&& func)
{
    canWriteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setReadCompleteHandler(
    TFunc&& func)
{
    readCompleteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setWriteCompleteHandler(
    TFunc&& func)
{
    writeCompleteHandler_ = std::forward<TFunc>(func);
}


template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
startRead(std::size_t length, EventLoopContext context)
{
    static_cast<void>(context);
    GASSERT(!isReadInterruptEnabled());
    GASSERT(remainingReadCount_ == 0);
    GASSERT(0 < length);
    remainingReadCount_ = length;
    setReadInterruptEnabled(true);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TContext>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
cancelRead(TContext context)
{
    static_cast<void>(context);
    return cancelReadInternal(); // The functionality is the same
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
startWrite(std::size_t length, EventLoopContext context)
{
    static_cast<void>(context);
    GASSERT(!isWriteInterruptEnabled());
    GASSERT(remainingWriteCount_ == 0);
    GASSERT(0 < length);
    remainingWriteCount_ = length;
    setWriteInterruptEnabled(true);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
cancelWrite(EventLoopContext context)
{
    static_cast<void>(context);
    setWriteInterruptEnabled(false);
    bool result = (0 < remainingWriteCount_);
    remainingWriteCount_ = 0;
    return result;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
canRead(InterruptContext context)
{
    static_cast<void>(context);
    return (((*pAUX_MU_LSR_REG & genMask(DataReadyPos)) != 0) &&
            ((0 < remainingReadCount_)));
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
canWrite(InterruptContext context)
{
    static_cast<void>(context);
    return (((*pAUX_MU_LSR_REG & genMask(TransmitterEmptyPos)) != 0) &&
            (0 < remainingWriteCount_));
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::CharType
Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
read(InterruptContext context)
{
    static_cast<void>(context);
    GASSERT(canRead(context));
    --remainingReadCount_;
    return static_cast<CharType>(
        *pAUX_MU_IO_REG & genMask(IoDataPos, IoDataLen));
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
write(CharType value, InterruptContext context)
{
    static_cast<void>(context);
    GASSERT(canWrite(context));
    --remainingWriteCount_;
    *pAUX_MU_IO_REG =
        static_cast<EntryType>(value) & genMask(IoDataPos, IoDataLen);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
cancelReadInternal()
{
    setReadInterruptEnabled(false);
    bool result = (0 < remainingReadCount_);
    remainingReadCount_ = 0;
    return result;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setReadInterruptEnabled(
    bool enabled)
{
    setBits(pAUX_MU_IER_REG, pAUX_MU_IER_REG_UsedBits, enabled, EnableRxInterruptPos);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
setWriteInterruptEnabled(
    bool enabled)
{
    setBits(pAUX_MU_IER_REG, pAUX_MU_IER_REG_UsedBits, enabled, EnableTxInterruptPos);
}


template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void Uart1<TInterruptMgr, TCanDoHandler,  TOpCompleteHandler>::
interruptHandler()
{
    if (((*pAUX_MU_IIR_REG & genMask(RxInterruptPos)) != 0) &&
         (isReadInterruptEnabled())) {

        if (canRead(InterruptContext())) {
            GASSERT(canReadHandler_);
            canReadHandler_();
        }

        if (remainingReadCount_ == 0) {
            setReadInterruptEnabled(false);
            GASSERT(readCompleteHandler_);
            readCompleteHandler_(embxx::error::ErrorCode::Success);
        }
    }

    if (((*pAUX_MU_IIR_REG & genMask(TxInterruptPos)) != 0) &&
        (isWriteInterruptEnabled())) {

        if (canWrite(InterruptContext())) {
            GASSERT(canWriteHandler_);
            canWriteHandler_();
        }

        if (remainingWriteCount_ == 0) {
            setWriteInterruptEnabled(false);
            GASSERT(writeCompleteHandler_);
            writeCompleteHandler_(embxx::error::ErrorCode::Success);
        }
    }
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
isReadInterruptEnabled()
{
    return ((*pAUX_MU_IER_REG & genMask(EnableRxInterruptPos)) != 0);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool Uart1<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
isWriteInterruptEnabled()
{
    return ((*pAUX_MU_IER_REG & genMask(EnableTxInterruptPos)) != 0);
}

}  // namespace device


