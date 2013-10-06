//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#pragma once

#include <cstdint>

#include "embxx/util/Assert.h"
#include "embxx/util/StaticFunction.h"

#include "Function.h"

namespace device
{

template <typename TInterruptMgr,
          typename TCanReadHandler = embxx::util::StaticFunction<void ()>,
          typename TCanWriteHandler = embxx::util::StaticFunction<void ()> >
class Uart1
{
public:

    typedef char CharType;

    typedef TInterruptMgr InterruptMgr;
    typedef TCanReadHandler CanReadHandler;
    typedef TCanWriteHandler CanWriteHandler;

    typedef typename InterruptMgr::IrqId IrqId;

    Uart1(InterruptMgr& interruptMgr, Function& funcDev, unsigned sysClock);

    static void setReadEnabled(bool enabled);
    static void setWriteEnabled(bool enabled);
    void configBaud(unsigned baud);

    static void setReadInterruptEnabled(bool enabled);
    static void setWriteInterruptEnabled(bool enabled);
    static bool canRead();
    static bool canWrite();
    static CharType read();
    static void write(CharType value);

    template <typename TFunc>
    void setCanReadHandler(TFunc&& func);

    template <typename TFunc>
    void setCanWriteHandler(TFunc&& func);

private:
    void interruptHandler();

    unsigned sysClock_;
    CanReadHandler canReadHandler_;
    CanWriteHandler canWriteHandler_;

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
          typename TCanReadHandler,
          typename TCanWriteHandler>
Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::Uart1(
    InterruptMgr& interruptMgr,
    Function& funcDev,
    unsigned sysClock)
    : sysClock_(sysClock)
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
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::setReadEnabled(
    bool enabled)
{
    setBits(pAUX_MU_CNTL_REG, pAUX_MU_CNTL_REG_UsedBits, enabled, ReceiverEnablePos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::setWriteEnabled(
    bool enabled)
{
    setBits(pAUX_MU_CNTL_REG, pAUX_MU_CNTL_REG_UsedBits, enabled, TransmitterEnablePos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::configBaud(
    unsigned baud)
{
    auto regValue = ((sysClock_ / baud) / 8) - 1;
    GASSERT(regValue <= genMask(BaudRatePos, BaudRateLen));
    *pAUX_MU_BAUD_REG = static_cast<EntryType>(regValue);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::setReadInterruptEnabled(
    bool enabled)
{
    setBits(pAUX_MU_IER_REG, pAUX_MU_IER_REG_UsedBits, enabled, EnableRxInterruptPos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::setWriteInterruptEnabled(
    bool enabled)
{
    setBits(pAUX_MU_IER_REG, pAUX_MU_IER_REG_UsedBits, enabled, EnableTxInterruptPos);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
bool Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::canRead()
{
    return ((*pAUX_MU_LSR_REG & genMask(DataReadyPos)) != 0);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
bool Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::canWrite()
{
    return ((*pAUX_MU_LSR_REG & genMask(TransmitterEmptyPos)) != 0);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
typename Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::CharType
Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::read()
{
    GASSERT(canRead());
    return static_cast<CharType>(
        *pAUX_MU_IO_REG & genMask(IoDataPos, IoDataLen));
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::write(
    CharType value)
{
    GASSERT(canWrite());
    *pAUX_MU_IO_REG =
        static_cast<EntryType>(value) & genMask(IoDataPos, IoDataLen);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
template <typename TFunc>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::setCanReadHandler(
    TFunc&& func)
{
    canReadHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
template <typename TFunc>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::setCanWriteHandler(
    TFunc&& func)
{
    canWriteHandler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          typename TCanReadHandler,
          typename TCanWriteHandler>
void Uart1<TInterruptMgr, TCanReadHandler, TCanWriteHandler>::interruptHandler()
{
    if (((*pAUX_MU_IER_REG & genMask(EnableRxInterruptPos)) != 0) &&
         ((*pAUX_MU_IIR_REG & genMask(RxInterruptPos)) != 0) &&
         (canRead())) {
        if (canReadHandler_) {
            canReadHandler_();
        }
        else {
           // Clear the RX queue
           *pAUX_MU_IIR_REG =
               genMask(FifoEnablePos, FifoEnableLen) |
               genMask(RxInterruptPos);
           setReadInterruptEnabled(false);
        }
    }

    if (((*pAUX_MU_IER_REG & genMask(EnableTxInterruptPos)) != 0) &&
        ((*pAUX_MU_IIR_REG & genMask(TxInterruptPos)) != 0) &&
        (canWrite())) {
        if (canWriteHandler_) {
            canWriteHandler_();
        }
        else {
            setWriteInterruptEnabled(false);
        }
    }
}

}  // namespace device


