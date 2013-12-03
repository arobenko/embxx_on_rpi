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
#include "embxx/comms/field/BasicIntValue.h"

#include "MsgId.h"

namespace message
{

enum HeartbeatMsgFieldIdx {
    HeartbeatMsgFieldIdx_SeqNum
};


template <typename TTraits>
struct HeartbeatMsgFields {
    typedef std::tuple<
        embxx::comms::field::BasicIntValue<std::uint16_t, TTraits>
    > Type;
};

template <typename TBase>
class HeartbeatMsg :
    public embxx::comms::MetaMessageBase<
        MsgId_Heartbeat,
        TBase,
        HeartbeatMsg<TBase>,
        typename HeartbeatMsgFields<typename TBase::Traits>::Type
    >
{
    typedef
        embxx::comms::MetaMessageBase<
            MsgId_Heartbeat,
            TBase,
            HeartbeatMsg<TBase>,
            typename HeartbeatMsgFields<typename TBase::Traits>::Type
        > Base;
public:
    typedef typename Base::Traits Traits;
    typedef typename Base::Fields Fields;

    typedef typename std::tuple_element<HeartbeatMsgFieldIdx_SeqNum, Fields>::type SeqNumField;

    HeartbeatMsg() = default;
    HeartbeatMsg(const HeartbeatMsg&) = default;
    HeartbeatMsg(const Fields& fields);
    ~HeartbeatMsg() = default;

    HeartbeatMsg& operator=(const HeartbeatMsg&) = default;
};

// Implementation
template <typename TBase>
HeartbeatMsg<TBase>::HeartbeatMsg(const Fields& fields)
    : Base(fields)
{
}

}  // namespace message
