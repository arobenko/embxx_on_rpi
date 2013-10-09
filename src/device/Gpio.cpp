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

#include "Gpio.h"

#include <cstdint>
#include <cstddef>

#include <embxx/util/Assert.h>

namespace device
{

namespace
{

typedef std::uint32_t SingleWordType;
const std::size_t BitsInSingleWord = sizeof(SingleWordType) * 8;
const std::size_t NumOfWordsInBundle =
        ((Function::NumOfLines - 1) / BitsInSingleWord) + 1;

const Function::FuncSel DirToFuncSel[Gpio::Dir_NumOfDirs] =
{
    Function::FuncSel::Input,
    Function::FuncSel::Output
};

struct WordsBundle
{
    volatile SingleWordType entries[NumOfWordsInBundle];
};

auto const pGPSET = reinterpret_cast<WordsBundle*>(0x2020001C);
auto const pGPCLR = reinterpret_cast<WordsBundle*>(0x20200028);
auto const pGPLEV = reinterpret_cast<const WordsBundle*>(0x20200034);

const volatile SingleWordType* idxToEntry(
    Gpio::PinIdxType idx,
    const WordsBundle* bundle)
{
    GASSERT(idx < Function::NumOfLines);
    Gpio::PinIdxType modifiedIdx = idx;
    std::size_t entryIdx = 0;
    while (BitsInSingleWord <= modifiedIdx) {
        ++entryIdx;
        modifiedIdx -= BitsInSingleWord;
    }
    GASSERT(entryIdx < NumOfWordsInBundle);
    return &bundle->entries[entryIdx];
}

volatile SingleWordType* idxToEntry(Gpio::PinIdxType idx, WordsBundle* bundle)
{
    return
        const_cast<volatile SingleWordType*>(
            idxToEntry(idx, static_cast<const WordsBundle*>(bundle)));
}

SingleWordType idxToEntryBitmask(Gpio::PinIdxType idx)
{
    GASSERT(idx < Function::NumOfLines);
    Gpio::PinIdxType modifiedIdx = idx;
    while (BitsInSingleWord <= modifiedIdx) {
        modifiedIdx -= BitsInSingleWord;
    }
    return static_cast<SingleWordType>(1) << modifiedIdx;
}

void updateEntry(Gpio::PinIdxType idx, WordsBundle* bundle)
{
    auto entry = idxToEntry(idx, bundle);
    *entry |= idxToEntryBitmask(idx);
}

}  // namespace

Gpio::Gpio(Function& func)
    : func_(func)
{
}

void Gpio::configDir(PinIdxType idx, Dir dir)
{
    GASSERT(dir < Dir_NumOfDirs);
    auto funcSel = DirToFuncSel[dir];
    func_.configure(idx, funcSel);
}

void Gpio::writePin(PinIdxType idx, bool value)
{
    if (value) {
        updateEntry(idx, pGPSET);
        return;
    }
    updateEntry(idx, pGPCLR);
}

bool Gpio::readPin(PinIdxType idx) const
{
    SingleWordType entry = *idxToEntry(idx, pGPLEV);
    return entry != 0;
}

}  // namespace device


