//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#pragma once

#include "Gpio.h"

namespace device
{

class Led
{
public:
    Led(Gpio& gpio);

    void on();
    void off();

private:
    Gpio& gpio_;
};

}  // namespace device


