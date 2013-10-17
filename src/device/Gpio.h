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
          std::size_t TMaxNumOfHandlers = 0,
          typename THandler = embxx::util::StaticFunction<void ()> >
class Gpio
{
public:

    typedef TInterruptMgr InterruptMgr;

    static const std::size_t MaxNumOfHandlers = TMaxNumOfHandlers;

    typedef THandler Handler;

    typedef Function::PinIdxType PinIdxType;
    static const std::size_t NumOfLines = Function::NumOfLines;

    enum Dir {
        Dir_Input,
        Dir_Output,
        Dir_NumOfDirs // Must be last
    };

    explicit Gpio(InterruptMgr& interruptMgr, Function& func);

    void configDir(PinIdxType idx, Dir dir);

    void writePin(PinIdxType idx, bool value);

    bool readPin(PinIdxType idx) const;

    void setEdgeInterruptEnabled(
        PinIdxType idx,
        bool edgeFinalValue, // true for raising, false for falling
        bool enabled);

    template <typename TFunc>
    void setHandler(PinIdxType idx, TFunc&& func);

private:

    struct Node
    {
        Node()
            : pinIdx_(std::numeric_limits<PinIdxType>::max())
        {
        }

        PinIdxType pinIdx_;
        Handler handler_;
    };

    typedef std::uint32_t SingleWordType;
    static const std::size_t BitsInSingleWord = sizeof(SingleWordType) * 8;
    static const std::size_t NumOfWordsInBundle =
            ((Function::NumOfLines - 1) / BitsInSingleWord) + 1;

    struct WordsBundle
    {
        volatile SingleWordType entries[NumOfWordsInBundle];
    };

    static const volatile SingleWordType* idxToEntry(
        PinIdxType idx,
        const WordsBundle* bundle);

    static volatile SingleWordType* idxToEntry(
        PinIdxType idx,
        WordsBundle* bundle);

    static SingleWordType idxToEntryBitmask(PinIdxType idx);

    static void updateEntry(
        PinIdxType idx,
        WordsBundle* bundle,
        bool value = true);

    Node* allocHandlerSpace(PinIdxType idx);

    void interruptHandler();

    Function& func_;
    std::array<Node, MaxNumOfHandlers> interruptHandlers_;
    std::size_t interruptHandlersCount_;


    static const Function::FuncSel DirToFuncSel[Gpio::Dir_NumOfDirs];

    static constexpr WordsBundle* pGPSET =
        reinterpret_cast<WordsBundle*>(0x2020001C);
    static constexpr WordsBundle* pGPCLR =
        reinterpret_cast<WordsBundle*>(0x20200028);
    static constexpr WordsBundle* pGPLEV =
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
          std::size_t TMaxNumOfHandlers,
          typename THandler>
Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::Gpio(
    InterruptMgr& interruptMgr,
    Function& func)
    : func_(func),
      interruptHandlersCount_(0)
{
    if (0 < TMaxNumOfHandlers) {
        static const int NumOfInterrupts =
            (InterruptMgr::IrqId_Gpio4 - InterruptMgr::IrqId_Gpio1) + 1;

        for (int i = 0; i < NumOfInterrupts; ++i) {
            typedef typename InterruptMgr::IrqId IrqId;
            IrqId interruptIdx =
                static_cast<IrqId>(InterruptMgr::IrqId_Gpio1 + i);

            interruptMgr.registerHandler(
                interruptIdx,
                std::bind(&Gpio::interruptHandler, this));

            interruptMgr.enableInterrupt(interruptIdx);
        }
    }
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
void Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::configDir(
    PinIdxType idx,
    Dir dir)
{
    GASSERT(dir < Dir_NumOfDirs);
    auto funcSel = DirToFuncSel[dir];
    func_.configure(idx, funcSel);
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
void Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::writePin(
    PinIdxType idx,
    bool value)
{
    GASSERT(idx < NumOfLines);
    if (value) {
        updateEntry(idx, pGPSET);
        return;
    }
    updateEntry(idx, pGPCLR);
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
bool Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::readPin(PinIdxType idx) const
{
    GASSERT(idx < NumOfLines);
    SingleWordType entry = *idxToEntry(idx, pGPLEV);
    auto mask = idxToEntryBitmask(idx);
    return (entry & mask) != 0;
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
void Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::setEdgeInterruptEnabled(
    PinIdxType idx,
    bool edgeFinalValue,
    bool enabled)
{
    GASSERT(idx < NumOfLines);
    WordsBundle* bundle = pGPFEN;
    if (edgeFinalValue) {
        // rising edge
        bundle = pGPREN;
    }

    updateEntry(idx, bundle, enabled);
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
template <typename TFunc>
void Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::setHandler(
    PinIdxType idx,
    TFunc&& func)
{
    GASSERT(idx < NumOfLines);
    static_assert(0 < MaxNumOfHandlers,
        "Can't set handler when number of available spaces is zero");

    auto node = allocHandlerSpace(idx);
    node->handler_ = std::forward<TFunc>(func);
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
const Function::FuncSel Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::DirToFuncSel[Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::Dir_NumOfDirs] =
{
    Function::FuncSel::Input,
    Function::FuncSel::Output
};

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
const volatile typename Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::SingleWordType*
Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::idxToEntry(
        Gpio::PinIdxType idx,
        const WordsBundle* bundle)
{
    GASSERT(idx < Function::NumOfLines);
    PinIdxType modifiedIdx = idx;
    std::size_t entryIdx = 0;
    while (BitsInSingleWord <= modifiedIdx) {
        ++entryIdx;
        modifiedIdx -= BitsInSingleWord;
    }
    GASSERT(entryIdx < NumOfWordsInBundle);
    return &bundle->entries[entryIdx];
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
volatile typename Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::SingleWordType*
Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::idxToEntry(
    PinIdxType idx,
    WordsBundle* bundle)
{
    return
        const_cast<volatile SingleWordType*>(
            idxToEntry(idx, static_cast<const WordsBundle*>(bundle)));
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
typename Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::SingleWordType
Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::idxToEntryBitmask(
    PinIdxType idx)
{
    GASSERT(idx < Function::NumOfLines);
    Gpio::PinIdxType modifiedIdx = idx;
    while (BitsInSingleWord <= modifiedIdx) {
        modifiedIdx -= BitsInSingleWord;
    }
    return static_cast<SingleWordType>(1) << modifiedIdx;
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
void Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::updateEntry(
    PinIdxType idx,
    WordsBundle* bundle,
    bool value)
{
    auto entry = idxToEntry(idx, bundle);
    auto mask = idxToEntryBitmask(idx);
    if (value) {
        *entry |= mask;
    }
    else {
        *entry &= ~mask;
    }
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
typename Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::Node*
Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::allocHandlerSpace(
    PinIdxType idx)
{
    GASSERT(interruptHandlersCount_ < MaxNumOfHandlers);
    auto beginIter = &interruptHandlers_[0];
    auto endIter = &interruptHandlers_[interruptHandlersCount_];
    auto iter = std::find_if(beginIter, endIter,
        [idx](Node& node)
        {
            return (idx == node.pinIdx_);
        });

    if (iter == endIter) {
        // Doesn't exist yet
        ++interruptHandlersCount_;
        iter->pinIdx_ = idx;
    }
    return iter;
}

template <typename TInterruptMgr,
          std::size_t TMaxNumOfHandlers,
          typename THandler>
void Gpio<TInterruptMgr, TMaxNumOfHandlers, THandler>::interruptHandler()
{
    WordsBundle bundle = *pGPEDS;
    *pGPEDS = bundle; // clear all the reported interrupts

    auto endIter = &interruptHandlers_[interruptHandlersCount_];
    for (auto iter = &interruptHandlers_[0]; iter != endIter; ++iter) {
        auto* entry = idxToEntry(iter->pinIdx_, &bundle);
        auto mask = idxToEntryBitmask(iter->pinIdx_);
        if (((*entry & mask) != 0) && (iter->handler_)) {
            iter->handler_();
        }
    }
}

}  // namespace device


