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


#include "Led.h"

namespace component
{

template <typename TGpio>
class OnBoardLed : public Led<TGpio, false>
{
    typedef Led<TGpio, false> Base;
public:
    typedef typename Base::Gpio Gpio;
    typedef typename Base::PinIdType PinIdType;

    OnBoardLed(Gpio& gpio)
        : Base(gpio, OnBoardLedPinIdx, InitialyOn)
    {
    }

private:
    static const PinIdType OnBoardLedPinIdx = 16;
    static const bool InitialyOn = false;
};

}  // namespace component
