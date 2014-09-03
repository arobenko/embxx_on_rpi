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

#include "embxx/util/StaticFunction.h"

namespace component
{

/// @brief Button definition class.
/// @tparam TDriver Driver class
/// @tparam TActiveState Boolean value stating the value of GPIO line when
///         button is pressed.
template <typename TDriver,
          bool TActiveState,
          typename THandler = embxx::util::StaticFunction<void ()> >
class Button
{
public:
    typedef TDriver Driver;
    typedef typename Driver::Device Gpio;
    typedef typename Gpio::PinIdType PinIdType;
    static const bool ActiveState = TActiveState;
    typedef THandler Handler;

    Button(Driver& driver, PinIdType pin)
      : driver_(driver),
        pin_(pin),
        state_(driver.device().readPin(pin))
    {
        auto& gpio = driver_.device();
        gpio.configDir(pin, Gpio::Dir_Input);
        gpio.configInputEdge(pin, Gpio::Edge_Rising, true);
        gpio.configInputEdge(pin, Gpio::Edge_Falling, true);

        driver.asyncReadCont(
            pin,
            [this](const embxx::error::ErrorStatus& es, bool state)
            {
                static_cast<void>(es);
                GASSERT(!es);

                if (state_ != state) {
                    state_ = state;
                    invokeHandler();
                }
            });
    }

    ~Button()
    {
        driver_.cancelReadContNoCallback(pin_);
    }

    bool isPressed() const
    {
        if (TActiveState) {
            return state_;
        }
        return !state_;
    }

    template <typename TFunc>
    void setPressedHandler(TFunc&& func)
    {
        pressedHandler_ = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setReleasedHandler(TFunc&& func)
    {
        releasedHandler_ = std::forward<TFunc>(func);
    }

private:
    void invokeHandler()
    {
        bool pressed = isPressed();
        if (pressed && pressedHandler_) {
            pressedHandler_();
        }
        else if ((!pressed) && releasedHandler_) {
            releasedHandler_();
        }
    }

    Driver& driver_;
    PinIdType pin_;
    bool state_;
    Handler pressedHandler_;
    Handler releasedHandler_;
};

}  // namespace component



