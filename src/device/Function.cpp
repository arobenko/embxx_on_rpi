//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

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

static_assert(&pSel->entries[0] == reinterpret_cast<volatile SingleSelWord*>(0x20200000),
    "Select entry address is not as expected");
static_assert(&pSel->entries[1] == reinterpret_cast<volatile SingleSelWord*>(0x20200004),
    "Select entry address is not as expected");
static_assert(&pSel->entries[2] == reinterpret_cast<volatile SingleSelWord*>(0x20200008),
    "Select entry address is not as expected");
static_assert(&pSel->entries[3] == reinterpret_cast<volatile SingleSelWord*>(0x2020000C),
    "Select entry address is not as expected");
static_assert(&pSel->entries[4] == reinterpret_cast<volatile SingleSelWord*>(0x20200010),
    "Select entry address is not as expected");
static_assert(&pSel->entries[5] == reinterpret_cast<volatile SingleSelWord*>(0x20200014),
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


