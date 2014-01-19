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
#include "embxx/device/context.h"
#include "embxx/device/op_category.h"

#include "Function.h"

namespace device
{

template <typename TInterruptMgr,
          typename TCanDoHandler = embxx::util::StaticFunction<void ()>,
          typename TOpCompleteHandler = embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&)> >
class I2C0
{
public:

    typedef std::uint8_t CharType;
    typedef std::uint8_t DeviceIdType; // Currently only 7 bit addresses are supported
    typedef embxx::device::op_category::SequentialReadWrite OpCategory;

    typedef TInterruptMgr InterruptMgr;
    typedef TCanDoHandler CanReadHandler;
    typedef TCanDoHandler CanWriteHandler;
    typedef TOpCompleteHandler ReadCompleteHandler;
    typedef TOpCompleteHandler WriteCompleteHandler;
    typedef std::uint32_t EntryType;

    typedef embxx::device::context::EventLoop EventLoopContext;
    typedef embxx::device::context::Interrupt InterruptContext;

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

    template <typename TContext>
    void startRead(
        DeviceIdType address,
        std::size_t length,
        TContext contex);

    template <typename TContext>
    bool cancelRead(TContext context);

    template <typename TContext>
    void startWrite(
        DeviceIdType address,
        std::size_t length,
        TContext context);

    template <typename TContext>
    bool cancelWrite(TContext context);

    bool suspend(EventLoopContext context);
    void resume(EventLoopContext context);

    bool canRead(InterruptContext context);
    bool canWrite(InterruptContext context);
    CharType read(InterruptContext context);
    void write(CharType value, InterruptContext context);


private:
    enum class OpType {
        Idle,
        Read,
        Write
    };

    typedef std::uint16_t LengthType;

    void startReadInternal(DeviceIdType address, std::size_t length);
    bool cancelReadInternal();
    void startWriteInternal(DeviceIdType address, std::size_t length);
    bool cancelWriteInternal();
    void interruptHandler();
    void completeTransfer(const embxx::error::ErrorStatus& status);
    void setAddrAndLen(DeviceIdType address, LengthType length);

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
    static const std::size_t BSC0_C_ReadTransferPos = 0;
    static const std::size_t BSC0_C_ClearFifoPos = 5;
    static const std::size_t BSC0_C_StartTransferPos = 7;
    static const std::size_t BSC0_C_InterruptOnDonePos = 8;
    static const std::size_t BSC0_C_InterruptOnTxPos = 9;
    static const std::size_t BSC0_C_InterruptOnRxPos = 10;
    static const std::size_t BSC0_C_I2CEnablePos = 15;

    static constexpr auto pBSC0_S =
        reinterpret_cast<volatile EntryType*>(0x20205004);
    static const std::size_t BSC0_S_TransferDonePos = 1;
    static const std::size_t BSC0_S_FifoNeedsWritingPos = 2;
    static const std::size_t BSC0_S_FifoNeedsReadingPos = 3;
    static const std::size_t BSC0_S_FifoCanAcceptDataPos = 4;
    static const std::size_t BSC0_S_FifoContainsDataPos = 5;
    static const std::size_t BSC0_S_AckErrorPos = 8;
    static const std::size_t BSC0_S_ClockStretchTimeoutPos = 9;

    static const EntryType BSC0_S_WritableBits =
        genMask(BSC0_S_TransferDonePos) |
        genMask(BSC0_S_AckErrorPos) |
        genMask(BSC0_S_ClockStretchTimeoutPos);


    static constexpr auto pBSC0_DLEN =
        reinterpret_cast<volatile EntryType*>(0x20205008);
    static const std::size_t BSC0_DLEN_DataLengthPos = 0;
    static const std::size_t BSC0_DLEN_DataLengthLen = 16;

    static constexpr auto pBSC0_A =
        reinterpret_cast<volatile EntryType*>(0x2020500C);
    static const std::size_t BS0_A_SlaveAddressPos = 0;
    static const std::size_t BS0_A_SlaveAddressLen = 7;

    static constexpr auto pBSC0_FIFO =
        reinterpret_cast<volatile EntryType*>(0x20205010);
    static const std::size_t BSC0_FIFO_DataPos = 0;
    static const std::size_t BSC0_FIFO_DataLen = 8;

    static constexpr auto pBSC0_DIV =
        reinterpret_cast<volatile EntryType*>(0x20205014);
    static const std::size_t BSC0_DIV_ClockDividerPos = 0;
    static const std::size_t BSC0_DIV_ClockDividerLen = 16;

};

// Implementation
template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::I2C0(
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

    static const auto InitialControlReg =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_ClearFifoPos);

    *pBSC0_C = InitialControlReg;

    interruptMgr.enableInterrupt(IrqId::IrqId_I2C);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::EntryType
I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::getDivider()
{
    return *pBSC0_DIV & genMask(BSC0_DIV_ClockDividerPos, BSC0_DIV_ClockDividerLen);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::setDivider(
    EntryType value)
{
    *pBSC0_DIV = value & genMask(BSC0_DIV_ClockDividerPos, BSC0_DIV_ClockDividerLen);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setCanReadHandler(
    TFunc&& func)
{
    canReadHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setCanWriteHandler(
    TFunc&& func)
{
    canWriteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setReadCompleteHandler(
    TFunc&& func)
{
    readCompleteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TFunc>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::
setWriteCompleteHandler(
    TFunc&& func)
{
    writeCompleteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TContext>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startRead(
    DeviceIdType address,
    std::size_t length,
    TContext context)
{
    static_cast<void>(context);
    startReadInternal(address, length);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TContext>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelRead(
    TContext context)
{
    static_cast<void>(context);
    return cancelReadInternal();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TContext>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startWrite(
    DeviceIdType address,
    std::size_t length,
    TContext context)
{
    static_cast<void>(context);
    startWriteInternal(address, length);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
template <typename TContext>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelWrite(
    TContext context)
{
    static_cast<void>(context);
    return cancelWriteInternal();
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::suspend(
    EventLoopContext context)
{
    static_cast<void>(context);

    auto suspendControl = genMask(BSC0_C_I2CEnablePos);
    if (op_ == OpType::Read) {
        suspendControl |= genMask(BSC0_C_ReadTransferPos);
    }

    *pBSC0_C = suspendControl;

    if (op_ == OpType::Idle) {
        *pBSC0_C = genMask(BSC0_C_I2CEnablePos);
        return false;
    }

    return true;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::resume(
    EventLoopContext context)
{
    static_cast<void>(context);

    auto resumeControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_InterruptOnDonePos);

    if (op_ == OpType::Read) {
        resumeControl |=
            (genMask(BSC0_C_ReadTransferPos) |
             genMask(BSC0_C_InterruptOnRxPos));
    }
    else {
        GASSERT(op_ == OpType::Write);
        resumeControl |= genMask(BSC0_C_InterruptOnTxPos);
    }

    *pBSC0_C = resumeControl;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::canRead(
    InterruptContext context)
{
    static_cast<void>(context);
    return ((*pBSC0_S & genMask(BSC0_S_FifoContainsDataPos)) != 0) &&
           (0 < remainingLen_) &&
           (op_ == OpType::Read);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::canWrite(
    InterruptContext context)
{
    static_cast<void>(context);
    return ((*pBSC0_S & genMask(BSC0_S_FifoCanAcceptDataPos)) != 0) &&
           (0 < remainingLen_) &&
           (op_ == OpType::Write);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
typename I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::CharType
I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::read(
    InterruptContext context)
{
    static_cast<void>(context);
    GASSERT(canRead(context));
    --remainingLen_;
    return static_cast<CharType>(
        *pBSC0_FIFO & genMask(BSC0_FIFO_DataPos, BSC0_FIFO_DataLen));
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::write(
    CharType value,
    InterruptContext context)
{
    static_cast<void>(context);
    GASSERT(canWrite(context));
    --remainingLen_;
    *pBSC0_FIFO =
        static_cast<EntryType>(value) &
        genMask(BSC0_FIFO_DataPos, BSC0_FIFO_DataLen);
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startReadInternal(
    DeviceIdType address,
    std::size_t length)
{
    GASSERT(op_ == OpType::Idle);
    GASSERT(remainingLen_ == 0);
    GASSERT(length <= genMask(BSC0_DLEN_DataLengthPos, BSC0_DLEN_DataLengthLen));
    op_ = OpType::Read;
    setAddrAndLen(address, static_cast<LengthType>(length));

    static const auto StartReadControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_InterruptOnRxPos) |
        genMask(BSC0_C_InterruptOnDonePos) |
        genMask(BSC0_C_ReadTransferPos) |
        genMask(BSC0_C_StartTransferPos) |
        genMask(BSC0_C_ClearFifoPos);

    *pBSC0_C = StartReadControl;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelReadInternal()
{
    static const auto SuspendReadControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_ReadTransferPos);

    *pBSC0_C = SuspendReadControl;

    if (op_ == OpType::Idle) {
        *pBSC0_C = genMask(BSC0_C_I2CEnablePos);
        return false;
    }

    GASSERT(op_ == OpType::Read);

    static const auto CancelReadControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_ClearFifoPos);

    *pBSC0_C = CancelReadControl;

    remainingLen_ = 0;
    op_ = OpType::Idle;
    return true;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::startWriteInternal(
    DeviceIdType address,
    std::size_t length)
{
    GASSERT(op_ == OpType::Idle);
    GASSERT(remainingLen_ == 0);
    GASSERT(length <= genMask(BSC0_DLEN_DataLengthPos, BSC0_DLEN_DataLengthLen));

    op_ = OpType::Write;
    setAddrAndLen(address, static_cast<LengthType>(length));

    static const auto StartWriteControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_InterruptOnTxPos) |
        genMask(BSC0_C_InterruptOnDonePos) |
        genMask(BSC0_C_StartTransferPos) |
        genMask(BSC0_C_ClearFifoPos);

    *pBSC0_C = StartWriteControl;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
bool I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::cancelWriteInternal()
{
    static const auto SuspendWriteControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_ReadTransferPos);

    *pBSC0_C = SuspendWriteControl;

    if (op_ == OpType::Idle) {
        *pBSC0_C = genMask(BSC0_C_I2CEnablePos);
        return false;
    }

    GASSERT(op_ == OpType::Write);
    static const auto CancelWriteControl =
        genMask(BSC0_C_I2CEnablePos) |
        genMask(BSC0_C_ClearFifoPos);

    *pBSC0_C = CancelWriteControl;

    remainingLen_ = 0;
    op_ = OpType::Idle;
    return true;
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::interruptHandler()
{
    EntryType status = *pBSC0_S;
    *pBSC0_S = status & BSC0_S_WritableBits; // clear some bits

    static const auto ErrorMask =
        genMask(BSC0_S_ClockStretchTimeoutPos) |
        genMask(BSC0_S_AckErrorPos);

    if (op_ == OpType::Idle) {
        // should not happen, but to increase robustness...
        static const auto IdleInterruptControl =
            genMask(BSC0_C_I2CEnablePos) |
            genMask(BSC0_C_ClearFifoPos);

        *pBSC0_C = IdleInterruptControl;
        return;
    }

    if ((status & ErrorMask) != 0) {
        completeTransfer(embxx::error::ErrorCode::HwProtocolError);
        return;
    }

    if ((status & genMask(BSC0_S_TransferDonePos)) &&
        ((*pBSC0_C & genMask(BSC0_C_InterruptOnDonePos)) != 0)) {

        if ((op_ == OpType::Read) &&
            (canRead(InterruptContext()))) {

            GASSERT(canReadHandler_);
            canReadHandler_();
        }

        if (0 < remainingLen_) {
            completeTransfer(embxx::error::ErrorCode::HwProtocolError);
            return;
        }

        completeTransfer(embxx::error::ErrorCode::Success);
        return;
    }

    if (((status & genMask(BSC0_S_FifoNeedsReadingPos)) != 0) &&
        ((*pBSC0_C & genMask(BSC0_C_InterruptOnRxPos)) != 0)) {
        GASSERT(canReadHandler_);
        canReadHandler_();

        if (remainingLen_ == 0) {
            static const auto WaitForReadCompleteControl =
                genMask(BSC0_C_I2CEnablePos) |
                genMask(BSC0_C_InterruptOnDonePos) |
                genMask(BSC0_C_ReadTransferPos);
            *pBSC0_C = WaitForReadCompleteControl;
        }
        return;
    }

    if (((status & genMask(BSC0_S_FifoNeedsWritingPos)) != 0) &&
        ((*pBSC0_C & genMask(BSC0_C_InterruptOnTxPos)) != 0)) {
        GASSERT(canWriteHandler_);
        canWriteHandler_();

        if (remainingLen_ == 0) {
            static const auto WaitForWriteCompleteControl =
                genMask(BSC0_C_I2CEnablePos) |
                genMask(BSC0_C_InterruptOnDonePos);

            *pBSC0_C = WaitForWriteCompleteControl;
        }
        return;
    }

    // Should not be here, spurious interrupt
}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::completeTransfer(
    const embxx::error::ErrorStatus& status)
{
    *pBSC0_C = genMask(BSC0_C_I2CEnablePos);

    auto opTmp = op_;
    op_ = OpType::Idle;
    if (opTmp == OpType::Read) {
        GASSERT(readCompleteHandler_);
        readCompleteHandler_(status);
    }
    else  {
        GASSERT(opTmp == OpType::Write);
        GASSERT(writeCompleteHandler_);
        writeCompleteHandler_(status);
    }

}

template <typename TInterruptMgr,
          typename TCanDoHandler,
          typename TOpCompleteHandler>
void I2C0<TInterruptMgr, TCanDoHandler, TOpCompleteHandler>::setAddrAndLen(
    DeviceIdType address,
    LengthType length)
{
    static_assert(
        std::numeric_limits<LengthType>::max() <= genMask(BSC0_DLEN_DataLengthPos, BSC0_DLEN_DataLengthLen),
        "Length type assumption is incorrect");

    GASSERT(address <= 0x7f);

    *pBSC0_DLEN = static_cast<EntryType>(length);
    *pBSC0_A = address & genMask(BS0_A_SlaveAddressPos, BS0_A_SlaveAddressLen);
    remainingLen_ = length;
}

}  // namespace device



