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

#include <tuple>
#include <cstdint>

#include "embxx/comms/Message.h"
#include "embxx/comms/field/BasicEnumValue.h"

#include "MsgId.h"

namespace message
{

enum ButtonStateChangeMsgFieldIdx {
    ButtonStateChangeMsgFieldIdx_State
};

enum class ButtonStateChangeMsgButtonState {
    Released,
    Pressed,
    NumOfStates
};


template <typename TTraits>
struct ButtonStateChangeMsgFields {
    typedef std::tuple<
        embxx::comms::field::BasicEnumValue<ButtonStateChangeMsgButtonState, TTraits, 1, ButtonStateChangeMsgButtonState::NumOfStates>
    > Type;
};

template <typename TBase>
class ButtonStateChangeMsg :
    public embxx::comms::MetaMessageBase<
        MsgId_ButtonStateChange,
        TBase,
        ButtonStateChangeMsg<TBase>,
        typename ButtonStateChangeMsgFields<typename TBase::Traits>::Type
    >
{
    typedef
        embxx::comms::MetaMessageBase<
            MsgId_ButtonStateChange,
            TBase,
            ButtonStateChangeMsg<TBase>,
            typename ButtonStateChangeMsgFields<typename TBase::Traits>::Type
        > Base;
public:
    typedef typename Base::Traits Traits;
    typedef typename Base::Fields Fields;

    typedef typename std::tuple_element<ButtonStateChangeMsgFieldIdx_State, Fields>::type StateField;

    ButtonStateChangeMsg() = default;
    ButtonStateChangeMsg(const ButtonStateChangeMsg&) = default;
    ButtonStateChangeMsg(const Fields& fields);
    ~ButtonStateChangeMsg() = default;

    ButtonStateChangeMsg& operator=(const ButtonStateChangeMsg&) = default;
};

// Implementation
template <typename TBase>
ButtonStateChangeMsg<TBase>::ButtonStateChangeMsg(const Fields& fields)
    : Base(fields)
{
}

}  // namespace message



