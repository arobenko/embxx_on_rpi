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

enum LedStateChangeMsgFieldIdx {
    LedStateChangeMsgFieldIdx_State
};

typedef LedState LedStateChangeMsgLedState;


template <typename TTraits>
struct LedStateChangeMsgFields {
    typedef std::tuple<
        embxx::comms::field::BasicEnumValue<LedStateChangeMsgLedState, TTraits, 1, LedStateChangeMsgLedState::NumOfStates>
    > Type;
};

template <typename TBase>
class LedStateChangeMsg :
    public embxx::comms::MetaMessageBase<
        MsgId_LedStateChange,
        TBase,
        LedStateChangeMsg<TBase>,
        typename LedStateChangeMsgFields<typename TBase::Traits>::Type
    >
{
    typedef
        embxx::comms::MetaMessageBase<
            MsgId_LedStateChange,
            TBase,
            LedStateChangeMsg<TBase>,
            typename LedStateChangeMsgFields<typename TBase::Traits>::Type
        > Base;
public:
    typedef typename Base::Traits Traits;
    typedef typename Base::Fields Fields;

    typedef typename std::tuple_element<LedStateChangeMsgFieldIdx_State, Fields>::type StateField;

    LedStateChangeMsg() = default;
    LedStateChangeMsg(const LedStateChangeMsg&) = default;
    LedStateChangeMsg(const Fields& fields);
    ~LedStateChangeMsg() = default;

    LedStateChangeMsg& operator=(const LedStateChangeMsg&) = default;
};

// Implementation
template <typename TBase>
LedStateChangeMsg<TBase>::LedStateChangeMsg(const Fields& fields)
    : Base(fields)
{
}

}  // namespace message






