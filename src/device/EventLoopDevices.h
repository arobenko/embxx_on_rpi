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
#include "InterruptMgr.h"

namespace device
{

class InterruptLock
{
public:
    InterruptLock()
        : flags_(0)
#ifndef NDEBUG
          , locked_(0)
#endif
    {}
    void lock()
    {
        GASSERT(!locked_);
        __asm volatile("mrs %0, cpsr" : "=r" (flags_));
        device::interrupt::disable();
#ifndef NDEBUG
        locked_ = true;
#endif
    }

    void unlock()
    {
        GASSERT(locked_);
        if ((flags_ & IntMask) == 0) {
            // Was previously enabled
            device::interrupt::enable();
        }
#ifndef NDEBUG
        locked_ = false;
#endif
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
    volatile std::uint32_t flags_;
#ifndef NDEBUG
    bool locked_;
#endif
    static const std::uint32_t IntMask = 1U << 7;
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
