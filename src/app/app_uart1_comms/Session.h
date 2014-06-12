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

#include <cstdint>
#include <iterator>
#include <tuple>

#include "embxx/comms/Message.h"
#include "embxx/comms/protocol.h"
#include "embxx/comms/protocol/checksum/BytesSum.h"
#include "embxx/comms/MsgAllocators.h"

#include "System.h"
#include "AllMsgsDefs.h"

struct CommsTraits {
    typedef embxx::comms::traits::endian::Big Endianness;
    typedef embxx::comms::traits::checksum::VerifyAfterProcessing ChecksumVerification;
    typedef System::CommsInStreamBuf::ConstIterator ReadIterator;
    typedef std::back_insert_iterator<System::CommsOutStreamBuf> WriteIterator;
    static const std::size_t MsgIdLen = 1;
    static const std::size_t MsgSizeLen = 1;
    static const std::size_t ChecksumLen = 2;
    static const std::size_t ExtraSizeValue = ChecksumLen;
    static const std::size_t ChecksumBase = 0;
    static const std::size_t SyncPrefixLen = 2;
};


class Session : public AllMsgsDefs<Session, CommsTraits>
{
public:

    typedef System::TimerMgr::Timer Timer;

    Session(System& system);
    ~Session() = default;

    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator=(const Session&) = delete;
    Session& operator=(Session&&) = delete;

    void handleMessage(const LedStateCtrlMsg& msg);
    void handleMessage(const MsgBase& msg);

private:
    typedef System::CommsInStreamBuf CommsInStreamBuf;
    typedef System::CommsOutStreamBuf CommsOutStreamBuf;

    typedef embxx::comms::protocol::MsgDataLayer<MsgBase> MsgDataLayer;
    typedef embxx::comms::protocol::MsgIdLayer<
        AllMsgs,
        embxx::comms::InPlaceMsgAllocator<AllMsgs>,
        CommsTraits,
        MsgDataLayer
    > MsgIdLayer;

    typedef embxx::comms::protocol::MsgSizeLayer<
        CommsTraits,
        MsgIdLayer
    > MsgSizeLayer;

    typedef embxx::comms::protocol::SyncPrefixLayer<
            CommsTraits,
            MsgSizeLayer
    > SyncPrefixLayer;

    typedef embxx::comms::protocol::ChecksumLayer<
            CommsTraits,
            embxx::comms::protocol::checksum::BytesSum<CommsTraits>,
            SyncPrefixLayer
        > ChecksumLayer;

    typedef ChecksumLayer ProtocolStack;
//    typedef SyncPrefixLayer ProtocolStack;
    typedef ProtocolStack::MsgPtr MsgPtr;

    void buttonStateChanged(message::ButtonStateChangeMsgButtonState state);
    void ledStateChanged(message::LedStateChangeMsgLedState state);
    void scheduleHeartbeat();
    void sendHeartbeat();
    void sendMessage(const MsgBase& msg);
    void startRead();
    void scheduleRead(std::size_t lenth);
    void readHandler(const embxx::error::ErrorStatus& status);

    System& system_;
    Timer heartbeatTimer_;
    ProtocolStack protStack_;
    HeartbeatMsg::SeqNumField::ValueType heartbeatSeqNumValue_;

    typedef SyncPrefixLayer::SyncPrefixType SyncPrefixType;
    static const SyncPrefixType SyncPrefixValue = 0x4869; // "Hi" in ascii
};


