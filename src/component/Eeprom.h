//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
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

#include <functional>
#include <utility>
#include <type_traits>

#include "embxx/util/StaticFunction.h"
#include "embxx/util/Assert.h"
#include "embxx/error/ErrorStatus.h"

namespace component
{

template <typename TDriver,
          typename THandler =
              embxx::util::StaticFunction<void (const embxx::error::ErrorStatus&, std::size_t)> >
class Eeprom
{
public:
    typedef TDriver Driver;
    typedef THandler Handler;

    typedef typename Driver::CharType CharType;
    typedef unsigned AttemtsCountType;
    typedef std::uint16_t EepromAddressType;
    typedef typename Driver::Device::DeviceIdType DeviceIdType;

    explicit Eeprom(Driver& driver);

    void setAttemptsLimit(AttemtsCountType limit);

    DeviceIdType getDeviceId() const;

    template <typename TFunc>
    void asyncRead(CharType* buf, std::size_t size, TFunc&& callback);

    template <typename TFunc>
    void asyncWrite(const CharType* buf, std::size_t size, TFunc&& callback);

private:
    void startOp();
    void opCompleteCallback(const embxx::error::ErrorStatus& err, std::size_t bytesTransferred);
    void invokeHandler(const embxx::error::ErrorStatus& err, std::size_t bytesTransferred);

    typedef embxx::util::StaticFunction<void ()> DriverCallerFunc;

    Driver& driver_;
    DriverCallerFunc driverCaller_;
    Handler handler_;
    AttemtsCountType attemptsLimit_;
    AttemtsCountType attempt_;
};

// Implementation

template <typename TDriver, typename THandler>
Eeprom<TDriver, THandler>::Eeprom(Driver& driver)
    : driver_(driver),
      attemptsLimit_(0),
      attempt_(0)
{
}

template <typename TDriver, typename THandler>
void Eeprom<TDriver, THandler>::setAttemptsLimit(AttemtsCountType limit)
{
    attemptsLimit_ = limit;
}

template <typename TDriver, typename THandler>
typename Eeprom<TDriver, THandler>::DeviceIdType
Eeprom<TDriver, THandler>::getDeviceId() const
{
    return driver_.device().id();
}

template <typename TDriver, typename THandler>
template <typename TFunc>
void Eeprom<TDriver, THandler>::asyncRead(
    CharType* buf,
    std::size_t size,
    TFunc&& callback)
{
    GASSERT(!handler_);
    GASSERT(!driverCaller_);
    handler_ = std::forward<TFunc>(callback);

    driverCaller_ =
        [this, buf, size]()
        {
            driver_.asyncRead(
                buf,
                size,
                std::bind(
                    &Eeprom::opCompleteCallback,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        };

    startOp();
}

template <typename TDriver, typename THandler>
template <typename TFunc>
void Eeprom<TDriver, THandler>::asyncWrite(
    const CharType* buf,
    std::size_t size,
    TFunc&& callback)
{
    GASSERT(!handler_);
    GASSERT(!driverCaller_);
    handler_ = std::forward<TFunc>(callback);
    driverCaller_ =
        [this, buf, size]()
        {
            driver_.asyncWrite(
                buf,
                size,
                std::bind(
                    &Eeprom::opCompleteCallback,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        };


    startOp();
}

template <typename TDriver, typename THandler>
void Eeprom<TDriver, THandler>::startOp()
{
    GASSERT(driverCaller_);
    attempt_ = 0;
    driverCaller_();
}

template <typename TDriver, typename THandler>
void Eeprom<TDriver, THandler>::opCompleteCallback(
    const embxx::error::ErrorStatus& err,
    std::size_t bytesTransferred)
{
    if (err == embxx::error::ErrorCode::HwProtocolError) {
        if (0 < attemptsLimit_) {
            ++attempt_;
            if (attemptsLimit_ <= attempt_) {
                invokeHandler(err, bytesTransferred);
                return;
            }
        }

        GASSERT(driverCaller_);
        driverCaller_();
        return;
    }

    invokeHandler(err, bytesTransferred);
}

template <typename TDriver, typename THandler>
void Eeprom<TDriver, THandler>::invokeHandler(
    const embxx::error::ErrorStatus& err,
    std::size_t bytesTransferred)
{
    GASSERT(handler_);
    driverCaller_ = nullptr;
    decltype(handler_) handlerCpy(std::move(handler_));
    handler_ = nullptr;
    GASSERT(!driverCaller_);
    GASSERT(!handler_);
    GASSERT(handlerCpy);
    handlerCpy(err, bytesTransferred);
}


}  // namespace component


