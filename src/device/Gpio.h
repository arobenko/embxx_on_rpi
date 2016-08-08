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
#include <cstddef>
#include <array>
#include <limits>
#include <algorithm>
#include <functional>

#include "embxx/util/StaticFunction.h"
#include <embxx/util/Assert.h>

#include "Function.h"

namespace device
{

template <typename TInterruptMgr,
          typename THandler = embxx::util::StaticFunction<void (Function::PinIdxType, bool)> >
class Gpio
{

public:

    typedef TInterruptMgr InterruptMgr;
    typedef THandler Handler;

    typedef embxx::device::context::EventLoop EventLoopCtx;

    typedef Function::PinIdxType PinIdType;
    static const std::size_t NumOfLines = Function::NumOfLines;

    enum Dir {
        Dir_Input,
        Dir_Output,
        Dir_NumOfDirs // Must be last
    };

    enum Edge {
        Edge_Rising,
        Edge_Falling,
        Edge_NumOfEdges // Must be last
    };

    Gpio(InterruptMgr& interruptMgr, Function& func)
      : interruptMgr_(interruptMgr),
        func_(func),
        enabled_(false)
    {
        for (auto& c : edgeConfig) {
            c = 0U;
        }

        for (int i = 0; i < NumOfInterrupts; ++i) {
            typedef typename InterruptMgr::IrqId IrqId;
            IrqId interruptIdx =
                static_cast<IrqId>(InterruptMgr::IrqId_Gpio1 + i);

            interruptMgr.registerHandler(
                interruptIdx,
                [this](){
                    WordsBundle bundle = *pGPEDS;
                    *pGPEDS = bundle; // clear all the reported interrupts
                    for (PinIdType id = 0U; id < NumOfLines; ++id) {
                        auto* entry = idxToEntry(id, &bundle);
                        auto mask = idxToEntryBitmask(id);
                        if ((*entry & mask) == 0) {
                            continue;
                        }

                        typedef typename EdgeConfigData::value_type EdgConfigValuetype;
                        auto value = readPin(id);
                        auto edgeConfigMask =
                            static_cast<EdgConfigValuetype>(1) << id;
                        GASSERT(handler_);
                        if (((value) && ((edgeConfig[Edge_Rising] & edgeConfigMask) != 0)) ||
                            ((!value) && ((edgeConfig[Edge_Falling] & edgeConfigMask) != 0))) {
                            handler_(id, value);
                        }
                    }
                });
            setInterruptsEnabled(false);
        }
    }

    void configDir(PinIdType pin, Dir dir)
    {
        GASSERT(dir < Dir_NumOfDirs);
        auto funcSel = DirToFuncSel[dir];
        func_.configure(pin, funcSel);
    }

    void configInputEdge(
        PinIdType pin,
        Edge edge,
        bool enabled)
    {
        if ((edge < static_cast<decltype(edge)>(0)) ||
            (Edge_NumOfEdges <= edge)) {
            GASSERT(!"Invalid edge value");
            return;
        }

        if (NumOfLines <= pin) {
            GASSERT(!"Invalid pin");
            return;
        }

        typedef typename EdgeConfigData::value_type EdgConfigValuetype;
        static_assert(NumOfLines <
            std::numeric_limits<EdgConfigValuetype>::digits,
            "Unexpected number of lines");

        auto mask = static_cast<EdgConfigValuetype>(1) << pin;
        if (enabled) {
            edgeConfig[edge] |= mask;
        }
        else {
            edgeConfig[edge] &= (~mask);
        }
    }

    void writePin(PinIdType pin, bool value)
    {
        GASSERT(pin < NumOfLines);
        if (value) {
            updateEntry(pin, pGPSET);
            return;
        }
        updateEntry(pin, pGPCLR);
    }

    bool readPin(PinIdType pin) const
    {
        GASSERT(pin < NumOfLines);
        SingleWordType entry = *idxToEntry(pin, pGPLEV);
        auto mask = idxToEntryBitmask(pin);
        return (entry & mask) != 0;
    }

    template <typename TFunc>
    void setHandler(TFunc&& func)
    {
        handler_ = std::forward<TFunc>(func);
    }

    void start(EventLoopCtx)
    {
        GASSERT(!enabled_);
        setInterruptsEnabled(true);
        enabled_ = true;
    }

    bool cancel(EventLoopCtx)
    {
        setInterruptsEnabled(false);
        if (!enabled_) {
            return false;
        }
        enabled_ = false;
        return true;
    }

    void setEnabled(PinIdType pin, bool enabled, EventLoopCtx)
    {
        GASSERT(pin < NumOfLines);
        if (NumOfLines <= pin) {
            return;
        }

        if (!enabled) {
            updateEntry(pin, pGPREN, enabled);
            updateEntry(pin, pGPFEN, enabled);
            return;
        }

        typedef typename EdgeConfigData::value_type EdgConfigValuetype;
        GASSERT(pin < std::numeric_limits<EdgConfigValuetype>::digits);
        auto mask = static_cast<EdgConfigValuetype>(1) << pin;
        if ((edgeConfig[Edge_Rising] & mask) != 0) {
            updateEntry(pin, pGPREN, enabled);
        }

        if ((edgeConfig[Edge_Falling] & mask) != 0) {
            updateEntry(pin, pGPFEN, enabled);
        }
    }

    bool suspend(EventLoopCtx)
    {
        setInterruptsEnabled(false);
        return enabled_;
    }

    void resume(EventLoopCtx) {
        GASSERT(enabled_);
        setInterruptsEnabled(true);
    }

private:

    typedef std::uint32_t SingleWordType;
    static const std::size_t BitsInSingleWord = sizeof(SingleWordType) * 8;
    static const std::size_t NumOfWordsInBundle =
            ((Function::NumOfLines - 1) / BitsInSingleWord) + 1;

    struct WordsBundle
    {
        volatile SingleWordType entries[NumOfWordsInBundle];
    };

    void setInterruptsEnabled(bool enabled)
    {
        for (int i = 0; i < NumOfInterrupts; ++i) {
            typedef typename InterruptMgr::IrqId IrqId;
            IrqId interruptIdx =
                static_cast<IrqId>(InterruptMgr::IrqId_Gpio1 + i);

            if (enabled) {
                interruptMgr_.enableInterrupt(interruptIdx);
            }
            else {
                interruptMgr_.disableInterrupt(interruptIdx);
            }
        }
    }

    static const volatile SingleWordType* idxToEntry(
        PinIdType pin,
        const WordsBundle* bundle)
    {
        GASSERT(pin < Function::NumOfLines);
        PinIdType modifiedIdx = pin;
        std::size_t entryIdx = 0;
        while (BitsInSingleWord <= modifiedIdx) {
            ++entryIdx;
            modifiedIdx -= BitsInSingleWord;
        }
        GASSERT(entryIdx < NumOfWordsInBundle);
        return &bundle->entries[entryIdx];
    }

    static volatile SingleWordType* idxToEntry(
        PinIdType pin,
        WordsBundle* bundle)
    {
        return
            const_cast<volatile SingleWordType*>(
                idxToEntry(pin, static_cast<const WordsBundle*>(bundle)));
    }

    static SingleWordType idxToEntryBitmask(PinIdType pin)
    {
        GASSERT(pin < Function::NumOfLines);
        Gpio::PinIdType modifiedIdx = pin;
        while (BitsInSingleWord <= modifiedIdx) {
            modifiedIdx -= BitsInSingleWord;
        }
        return static_cast<SingleWordType>(1) << modifiedIdx;
    }

    static void updateEntry(
        PinIdType pin,
        WordsBundle* bundle,
        bool value = true)
    {
        auto entry = idxToEntry(pin, bundle);
        auto mask = idxToEntryBitmask(pin);
        if (value) {
            *entry |= mask;
        }
        else {
            *entry &= ~mask;
        }
    }

    typedef std::array<std::uint64_t, Edge_NumOfEdges> EdgeConfigData;
    InterruptMgr& interruptMgr_;
    Function& func_;
    Handler handler_;
    EdgeConfigData edgeConfig;
    bool enabled_;

    static const Function::FuncSel DirToFuncSel[Gpio::Dir_NumOfDirs];

    static const int NumOfInterrupts =
        (InterruptMgr::IrqId_Gpio4 - InterruptMgr::IrqId_Gpio1) + 1;

    static constexpr WordsBundle* pGPSET =
        reinterpret_cast<WordsBundle*>(0x2020001C);
    static constexpr WordsBundle* pGPCLR =
        reinterpret_cast<WordsBundle*>(0x20200028);
    static constexpr const WordsBundle* pGPLEV =
        reinterpret_cast<const WordsBundle*>(0x20200034);
    static constexpr WordsBundle* pGPEDS =
        reinterpret_cast<WordsBundle*>(0x20200040);
    static constexpr WordsBundle* pGPREN =
        reinterpret_cast<WordsBundle*>(0x2020004C);
    static constexpr WordsBundle* pGPFEN =
        reinterpret_cast<WordsBundle*>(0x20200058);

};

// Implementation


template <typename TInterruptMgr,
          typename THandler>
const Function::FuncSel Gpio<TInterruptMgr, THandler>::DirToFuncSel[Gpio<TInterruptMgr, THandler>::Dir_NumOfDirs] =
{
    Function::FuncSel::Input,
    Function::FuncSel::Output
};

}  // namespace device


