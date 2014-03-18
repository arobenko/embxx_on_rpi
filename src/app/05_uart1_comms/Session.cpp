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

#include "Session.h"

#include <iterator>
#include <algorithm>

#include "embxx/util/Assert.h"

namespace
{

const System::TimerMgr::WaitTimeType HeartbeatPeriod = 2000; // 2 sec

}  // namespace

Session::Session(System& system)
    : system_(system),
      heartbeatTimer_(system_.timerMgr().allocTimer()),
      protStack_(SyncPrefixType(SyncPrefixValue)),
      heartbeatSeqNumValue_(0)
{
    GASSERT(heartbeatTimer_.isValid());
    scheduleHeartbeat();

    auto& button = system_.button();
    button.setPressedHandler(
        std::bind(
            &Session::buttonStateChanged,
            this,
            message::ButtonStateChangeMsgButtonState::Pressed));

    button.setReleasedHandler(
        std::bind(
            &Session::buttonStateChanged,
            this,
            message::ButtonStateChangeMsgButtonState::Released));

    system_.commsInStreamBuf().start();
    startRead();

}

void Session::handleMessage(const LedStateCtrlMsg& msg)
{
    static const auto StateIdx = message::LedStateCtrlMsgFieldIdx_State;
    auto stateField = std::get<StateIdx>(msg.getFields());
    auto stateValue = stateField.getValue();
    if (stateValue == message::LedStateChangeMsgLedState::Off) {
        system_.led().off();
        ledStateChanged(message::LedStateChangeMsgLedState::Off);
    }
    else {
        system_.led().on();
        ledStateChanged(message::LedStateChangeMsgLedState::On);
    }
}

void Session::handleMessage(const MsgBase& msg)
{
    static_cast<void>(msg);
}

void Session::buttonStateChanged(message::ButtonStateChangeMsgButtonState state)
{
    auto fields = ButtonStateChangeMsg::Fields(
        ButtonStateChangeMsg::StateField(state));
    ButtonStateChangeMsg msg(fields);
    sendMessage(msg);
}

void Session::ledStateChanged(message::LedStateChangeMsgLedState state)
{
    auto fields = LedStateChangeMsg::Fields(
        LedStateChangeMsg::StateField(state));
    LedStateChangeMsg msg(fields);
    sendMessage(msg);
}

void Session::scheduleHeartbeat()
{
    GASSERT(heartbeatTimer_.isValid());
    heartbeatTimer_.asyncWait(
        HeartbeatPeriod,
        [this](const embxx::error::ErrorStatus& err)
        {
            if (err) {
                return;
            }

            sendHeartbeat();
            scheduleHeartbeat();
        });
}

void Session::sendHeartbeat()
{
    auto fields = HeartbeatMsg::Fields(
        HeartbeatMsg::SeqNumField(heartbeatSeqNumValue_));

    HeartbeatMsg msg(fields);
    sendMessage(msg);
    ++heartbeatSeqNumValue_;
}

void Session::sendMessage(const MsgBase& msg)
{
    auto& buf = system_.commsOutStreamBuf();
    GASSERT(buf.empty());
    auto writeIter = std::back_inserter(buf);
    auto status = protStack_.write(msg, writeIter, buf.availableCapacity() - buf.size());
    if (status == embxx::comms::ErrorStatus::UpdateRequired) {
        auto updateIter = buf.begin();
        status = protStack_.update(updateIter, buf.size());
    }

    if (status != embxx::comms::ErrorStatus::Success) {
        // Message dropped
        buf.clear();
        return;
    }

    buf.flush();
}

void Session::startRead()
{
    scheduleRead(protStack_.length());
}

void Session::scheduleRead(std::size_t length)
{
    auto& buf = system_.commsInStreamBuf();
    buf.asyncWaitDataAvailable(
        length,
        std::bind(&Session::readHandler, this, std::placeholders::_1));
}

void Session::readHandler(const embxx::error::ErrorStatus& err)
{
    if (err.code() == embxx::error::ErrorCode::Aborted) {
        return;
    }

    if (err) {
        // Retry
        startRead();
        return;
    }

    auto& buf = system_.commsInStreamBuf();
    while (true) {
        if (buf.empty()) {
            startRead();
            return;
        }

        std::size_t missingSize = 0;
        auto readIter = buf.begin();
        MsgPtr msg;
        auto readStatus = protStack_.read(msg, readIter, buf.size(), &missingSize);
        if (readStatus == embxx::comms::ErrorStatus::NotEnoughData) {
            GASSERT(0 < missingSize);
            auto nextSize = std::min(buf.size() + missingSize, buf.fullCapacity());
            scheduleRead(nextSize);
            return;
        }

        if (readStatus != embxx::comms::ErrorStatus::Success) {
            buf.consume(1U);
            continue;
        }

        GASSERT(msg);
        auto lengthToConsume =
            static_cast<std::size_t>(std::distance(buf.begin(), readIter));
        GASSERT(0 < lengthToConsume);
        buf.consume(lengthToConsume);
        msg->dispatch(*this);
    }
}
