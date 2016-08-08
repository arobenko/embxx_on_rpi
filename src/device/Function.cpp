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

#include "Function.h"

#include <cstdint>
#include <cstddef>

#include <embxx/util/Assert.h>

namespace device
{

namespace
{

typedef std::uint32_t SingleSelWord;
const std::size_t NumOfBitsPerLine = 3;
const std::size_t BitsPerLineMask =
    (static_cast<SingleSelWord>(1) << NumOfBitsPerLine) - 1;
const std::size_t NumOfLinesPerSelWord = (sizeof(SingleSelWord) * 8) / NumOfBitsPerLine;
const std::size_t NumOfSelWords = ((Function::NumOfLines - 1) / NumOfLinesPerSelWord) + 1;

struct SelWords
{
    volatile SingleSelWord entries[NumOfSelWords];
};

volatile SelWords* const pSel = reinterpret_cast<SelWords*>(0x20200000);

static_assert(offsetof(SelWords, entries) == 0,
     "Select entry address is not as expected");
static_assert(offsetof(SelWords, entries[1]) == 4,
     "Select entry address is not as expected");
static_assert(offsetof(SelWords, entries[2]) == 8,
     "Select entry address is not as expected");
static_assert(offsetof(SelWords, entries[3]) == 12,
     "Select entry address is not as expected");
static_assert(offsetof(SelWords, entries[4]) == 16,
     "Select entry address is not as expected");
static_assert(offsetof(SelWords, entries[5]) == 20,
     "Select entry address is not as expected");

}  // namespace

void Function::configure(PinIdxType idx, FuncSel sel)
{
    std::size_t selIdx = idx / NumOfLinesPerSelWord;
    GASSERT(selIdx < NumOfSelWords);
    SingleSelWord selValue = pSel->entries[selIdx];

    GASSERT(static_cast<SingleSelWord>(sel) <
                        (static_cast<SingleSelWord>(1) << NumOfBitsPerLine));
    std::size_t selEntryIdx = idx % NumOfLinesPerSelWord;
    std::size_t selEntryShift = selEntryIdx * NumOfBitsPerLine;
    selValue &= ~(BitsPerLineMask << selEntryShift);
    selValue |= static_cast<SingleSelWord>(sel) << selEntryShift;
    pSel->entries[selIdx] = selValue;
}

}  // namespace device


