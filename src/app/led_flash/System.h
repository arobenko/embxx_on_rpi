//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

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

// Implementation

inline
device::Led& System::led()
{
    return led_;
}

