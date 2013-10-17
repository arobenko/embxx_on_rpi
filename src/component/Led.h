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

#include "device/Gpio.h"

namespace component
{

/// @brief Led definition class.
/// @tparam TGpio Gpio class
/// @tparam TOnState Boolean value stating the value of GPIO line to set
///         led on.
template <typename TGpio, bool TOnState>
class Led
{
public:
    typedef TGpio Gpio;
    typedef typename Gpio::PinIdxType PinIdxType;
    static const bool OnState = TOnState;

    Led(Gpio& gpio, PinIdxType pidIdx, bool isOn = false);

    void on();
    void off();
    bool isOn() const;

private:
    Gpio& gpio_;
    PinIdxType pin_;
    bool isOn_;
};

// Implementation
template <typename TGpio, bool TOnState>
Led<TGpio, TOnState>::Led(Gpio& gpio, PinIdxType pin, bool isOn)
    : gpio_(gpio),
      pin_(pin),
      isOn_(isOn)
{
    gpio_.configDir(pin, Gpio::Dir_Output);
}

template <typename TGpio, bool TOnState>
void Led<TGpio, TOnState>::on()
{
    gpio_.writePin(pin_, OnState);
    isOn_ = true;
}

template <typename TGpio, bool TOnState>
void Led<TGpio, TOnState>::off()
{
    gpio_.writePin(pin_, !OnState);
    isOn_ = false;
}

template <typename TGpio, bool TOnState>
bool Led<TGpio, TOnState>::isOn() const
{
    return isOn_;
}

}  // namespace component


