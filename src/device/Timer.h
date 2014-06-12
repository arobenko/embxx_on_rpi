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

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <chrono>

#include "embxx/util/Assert.h"
#include "embxx/util/StaticFunction.h"
#include "embxx/error/ErrorStatus.h"
#include "embxx/device/context.h"

namespace device
{

template <typename TInterruptMgr,
          typename THandler = embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&), 20> >
class Timer
{
public:
    typedef unsigned WaitTimeType;
    typedef std::chrono::duration<WaitTimeType, std::milli> WaitTimeUnitDuration;

    typedef TInterruptMgr InterruptMgr;
    typedef THandler HandlerFunc;

    Timer(InterruptMgr& interruptMgr);

    template <typename TFunc>
    void setHandler(TFunc&& handler);

    template <typename TContext>
    void startWait(WaitTimeType waitMs, TContext context);

    bool cancelWait(embxx::device::context::EventLoop context);

    bool suspendWait(embxx::device::context::EventLoop context);
    void resumeWait(embxx::device::context::EventLoop context);

    unsigned getElapsed(embxx::device::context::EventLoop context) const;

private:
    typedef std::uint32_t EntryType;
    typedef typename InterruptMgr::IrqId IrqId;

    void startWaitInternal(WaitTimeType waitMs);
    void enableInterrupts();
    void disableInterrupts();
    bool configWait(WaitTimeType millisecs);
    void interruptHandler();


    InterruptMgr& interruptMgr_;
    HandlerFunc handler_;
    bool waitInProgress_;

    static const unsigned SysClockFreq = 1000000; // 1 MHz - calculated by trial and error

    static constexpr volatile EntryType* const LoadReg =
        reinterpret_cast<volatile EntryType*>(0x2000B400);

    static constexpr const volatile EntryType* const ValueReg =
        reinterpret_cast<volatile EntryType*>(0x2000B404);

    static constexpr volatile EntryType* const ControlReg =
        reinterpret_cast<volatile EntryType*>(0x2000B408);

    static constexpr volatile EntryType* const IrqClearAckReg =
        reinterpret_cast<volatile EntryType*>(0x2000B40C);

    static constexpr const volatile EntryType* const RawIrqReg =
        reinterpret_cast<volatile EntryType*>(0x2000B410);

    static constexpr const volatile EntryType* const MaskedIrqReg =
        reinterpret_cast<volatile EntryType*>(0x2000B414);

    static constexpr volatile EntryType* const ReloadReg =
        reinterpret_cast<volatile EntryType*>(0x2000B418);

    static constexpr volatile EntryType* const PreDeviderReg =
        reinterpret_cast<volatile EntryType*>(0x2000B41C);

    static constexpr volatile EntryType* const FreeRunningCounterReg =
        reinterpret_cast<volatile EntryType*>(0x2000B420);

    // Control reg masks
    static const std::size_t ControlRegCounterTypePos = 1;
    static const EntryType ControlRegCounterTypeMask =
        static_cast<EntryType>(1) << ControlRegCounterTypePos;

    static const std::size_t ControlRegPrescalerPos = 2;
    static const EntryType ControlRegPrescalerMask =
        static_cast<EntryType>(3) << ControlRegPrescalerPos;

    static const std::size_t ControlRegIrqEnablePos = 5;
    static const EntryType ControlRegIrqEnableMask =
        static_cast<EntryType>(1) << ControlRegIrqEnablePos;

    static const std::size_t ControlRegTimerEnablePos = 7;
    static const EntryType ControlRegTimerEnableMask =
        static_cast<EntryType>(1) << ControlRegTimerEnablePos;
};

// Implementation
template <typename TInterruptMgr,
          typename THandler>
Timer<TInterruptMgr, THandler>::Timer(InterruptMgr& interruptMgr)
    : interruptMgr_(interruptMgr),
      waitInProgress_(false)
{
    // Make it 32 bit counter by default
    *ControlReg |= ControlRegCounterTypeMask;
    interruptMgr_.registerHandler(
        IrqId::IrqId_Timer,
        std::bind(&Timer::interruptHandler, this));
    *ControlReg &= ~ControlRegTimerEnableMask;
    disableInterrupts();
    *IrqClearAckReg = 1; // Clear the interrupt if such exists
}

template <typename TInterruptMgr,
          typename THandler>
template <typename TFunc>
void Timer<TInterruptMgr, THandler>::setHandler(TFunc&& handler)
{
    handler_ = std::forward<TFunc>(handler);
}

template <typename TInterruptMgr,
          typename THandler>
template <typename TContext>
void Timer<TInterruptMgr, THandler>::startWait(
    WaitTimeType waitMs,
    TContext context)
{
    static_cast<void>(context);
    startWaitInternal(waitMs);
}

template <typename TInterruptMgr,
          typename THandler>
bool Timer<TInterruptMgr, THandler>::cancelWait(
    embxx::device::context::EventLoop context)
{
    static_cast<void>(context);
    disableInterrupts();
    if (!waitInProgress_) {
        return false;
    }

    *ControlReg &= ~ControlRegTimerEnableMask;
    waitInProgress_ = false;
    return true;
}

template <typename TInterruptMgr,
          typename THandler>
bool Timer<TInterruptMgr, THandler>::suspendWait(
    embxx::device::context::EventLoop context)
{
    static_cast<void>(context);
    disableInterrupts();
    if (!waitInProgress_) {
        return false;
    }

    return true;
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::resumeWait(
    embxx::device::context::EventLoop context)
{
    static_cast<void>(context);
    GASSERT(waitInProgress_);
    enableInterrupts();
}

template <typename TInterruptMgr,
          typename THandler>
unsigned Timer<TInterruptMgr, THandler>::getElapsed(
    embxx::device::context::EventLoop context) const
{
    static_cast<void>(context);
    unsigned elapsedTicks = *LoadReg - *ValueReg;
    unsigned prescaler =
        (*ControlReg & ControlRegPrescalerMask) >> ControlRegPrescalerPos;

    while (0 < prescaler) {
        elapsedTicks <<= 4;
        --prescaler;
    }

    static const unsigned TicksInMillisec = (SysClockFreq / 1000);
    return elapsedTicks / TicksInMillisec;
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::startWaitInternal(
    WaitTimeType waitMs)
{
    GASSERT(!waitInProgress_);
    waitInProgress_ = true;
    configWait(waitMs);
    enableInterrupts();
    *ControlReg |= ControlRegTimerEnableMask;
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::enableInterrupts()
{
    *ControlReg |= ControlRegIrqEnableMask;
    interruptMgr_.enableInterrupt(IrqId::IrqId_Timer);
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::disableInterrupts()
{
    interruptMgr_.disableInterrupt(IrqId::IrqId_Timer);
    *ControlReg &= ~ControlRegIrqEnableMask;
}

template <typename TInterruptMgr,
          typename THandler>
bool Timer<TInterruptMgr, THandler>::configWait(unsigned millisecs)
{
    static const std::uint64_t TicksInMillisec = (SysClockFreq / 1000);
    static const unsigned MaxSupportedPrescaler = 2;

    auto totalTicks = TicksInMillisec * millisecs;
    unsigned prescaler = 0;

    while (std::numeric_limits<EntryType>::max() < totalTicks) {
        totalTicks >>= 4; // divide by 16;
        prescaler += 1;
    }

    if (MaxSupportedPrescaler < prescaler) {
        return false;
    }

    *LoadReg = totalTicks;
    *ControlReg &= (~ControlRegPrescalerMask);
    *ControlReg |= (static_cast<EntryType>(prescaler) << ControlRegPrescalerPos);

    return true;
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::interruptHandler()
{
    *IrqClearAckReg = 1; // Clear the interrupt
    waitInProgress_ = false;
    if (handler_) {
        handler_(embxx::error::ErrorCode::Success);
    }
}

}  // namespace device


