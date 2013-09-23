//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#pragma once

#include <cstdint>

#include "InterruptMgr.h"

namespace device
{

class InterruptLock
{
public:
    InterruptLock() : flags_(0)
    {}
    void lock()
    {
//        __asm volatile("mrs %0, cpsr" : "=r" (flags_));
        device::interrupt::disable();
    }

    void unlock()
    {
//        __asm volatile("msr cpsr, %0" : "=r" (flags_));
        device::interrupt::enable();
    }

    void lockInterruptCtx()
    {
        // Nothing to do
    }

    void unlockInterruptCtx()
    {
        // Nothing to do
    }
private:
    std::uint32_t flags_;
};

class WaitCond
{
public:
    template <typename TLock>
    void wait(TLock& lock)
    {
        static_cast<void>(lock);
        __asm volatile("wfi"); // probably has no effect
    }

    void notify()
    {
        // Nothing to do
    }
};

}  // namespace device
