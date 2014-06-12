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
#include "LedState.h"

namespace message
{

enum LedStateCtrlMsgFieldIdx {
    LedStateCtrlMsgFieldIdx_State
};

typedef LedState LedStateCtrlMsgLedState;


template <typename TTraits>
struct LedStateCtrlMsgFields {
    typedef std::tuple<
        embxx::comms::field::BasicEnumValue<
            LedStateCtrlMsgLedState, TTraits, 1, LedStateCtrlMsgLedState::NumOfStates>
    > Type;
};

template <typename TBase>
class LedStateCtrlMsg :
    public embxx::comms::MetaMessageBase<
        MsgId_LedStateCtrl,
        TBase,
        LedStateCtrlMsg<TBase>,
        typename LedStateCtrlMsgFields<typename TBase::Traits>::Type
    >
{
    typedef
        embxx::comms::MetaMessageBase<
            MsgId_LedStateCtrl,
            TBase,
            LedStateCtrlMsg<TBase>,
            typename LedStateCtrlMsgFields<typename TBase::Traits>::Type
        > Base;
public:
    typedef typename Base::Traits Traits;
    typedef typename Base::Fields Fields;

    typedef typename std::tuple_element<LedStateCtrlMsgFieldIdx_State, Fields>::type StateField;

    LedStateCtrlMsg() = default;
    LedStateCtrlMsg(const LedStateCtrlMsg&) = default;
    LedStateCtrlMsg(const Fields& fields);
    ~LedStateCtrlMsg() = default;

    LedStateCtrlMsg& operator=(const LedStateCtrlMsg&) = default;
};

// Implementation
template <typename TBase>
LedStateCtrlMsg<TBase>::LedStateCtrlMsg(const Fields& fields)
    : Base(fields)
{
}

}  // namespace message



