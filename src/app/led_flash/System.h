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

#include "device/Function.h"
#include "device/Gpio.h"
#include "device/Led.h"

class System
{
public:
    static System& instance();

    inline device::Led& led();

private:
    System();

    device::Function func_;
    device::Gpio gpio_;
    device::Led led_;
};

extern "C"
void interruptHandler();

// Implementation

inline
device::Led& System::led()
{
    return led_;
}

