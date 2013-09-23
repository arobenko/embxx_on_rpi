//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "embxx/util/StaticFunction.h"

namespace device
{

template <typename TInterruptMgr,
          typename THandler = embxx::util::StaticFunction<void (), 20> >
class Timer
{
public:
    typedef unsigned WaitTimeType;

    typedef TInterruptMgr InterruptMgr;
    typedef THandler HandlerFunc;

    Timer(InterruptMgr& interruptMgr);

    void enableInterrupts();
    void disableInterrupts();
    bool hasPendingInterrupt() const;
    void start();
    void stop();
    bool configWait(unsigned millisecs);
    unsigned getElapsed() const;

    template <typename TFunc>
    void setHandler(TFunc&& handler);


private:
    typedef std::uint32_t EntryType;
    typedef typename InterruptMgr::IrqId IrqId;

    void interruptHandler();


    InterruptMgr& interruptMgr_;
    HandlerFunc handler_;

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
    : interruptMgr_(interruptMgr)
{
    // Make it 32 bit counter by default
    *ControlReg |= ControlRegCounterTypeMask;
    interruptMgr_.registerHandler(
        IrqId::IrqId_Timer,
        std::bind(&Timer::interruptHandler, this));
    stop();
    disableInterrupts();
    *IrqClearAckReg = 1; // Clear the interrupt if such exists
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
bool Timer<TInterruptMgr, THandler>::hasPendingInterrupt() const
{
    return (*RawIrqReg != 0);
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::start()
{
    *ControlReg |= ControlRegTimerEnableMask;
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::stop()
{
    *ControlReg &= ~ControlRegTimerEnableMask;
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
unsigned Timer<TInterruptMgr, THandler>::getElapsed() const
{
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
template <typename TFunc>
void Timer<TInterruptMgr, THandler>::setHandler(TFunc&& handler)
{
    handler_ = std::forward<TFunc>(handler);
}

template <typename TInterruptMgr,
          typename THandler>
void Timer<TInterruptMgr, THandler>::interruptHandler()
{
    *IrqClearAckReg = 1; // Clear the interrupt
    if (handler_) {
        handler_();
    }
}

}  // namespace device


