//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#include "Led.h"

namespace device
{

namespace
{

const Gpio::PinIdxType LedLineIdx = 16;

}  // namespace

Led::Led(Gpio& gpio)
    : gpio_(gpio)
{
    gpio_.configDir(LedLineIdx, Gpio::Dir_Output);
}

void Led::on()
{
    gpio_.writePin(LedLineIdx, false);
}

void Led::off()
{
    gpio_.writePin(LedLineIdx, true);
}

bool Led::isOn() const
{
    return gpio_.readPin(LedLineIdx);
}

}  // namespace device


