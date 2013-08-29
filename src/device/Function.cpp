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
const std::size_t NumOfLinesPerSelWord = sizeof(SingleSelWord) / NumOfBitsPerLine;
const std::size_t NumOfSelWords = ((Function::NumOfLines - 1) / NumOfLinesPerSelWord) + 1;

struct SelWords
{
    volatile SingleSelWord entries[NumOfSelWords];
};

SelWords* const pSel = reinterpret_cast<SelWords*>(0x20200000);

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
    SingleSelWord mask = static_cast<SingleSelWord>(sel) << selEntryShift;
    selValue &= ~mask;
    selValue |= static_cast<SingleSelWord>(sel) << selEntryShift;
    pSel->entries[selIdx] = selValue;
}

}  // namespace device


