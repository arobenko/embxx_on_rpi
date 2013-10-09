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
