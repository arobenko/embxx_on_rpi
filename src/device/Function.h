//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#pragma once

#include <cstddef>

namespace device
{

class Function
{
public:
    enum FuncSel {
        FuncSel_Input,  // b000
        FuncSel_Output, // b001
        FuncSel_Alt5,   // b010
        FuncSel_Alt4,   // b011
        FuncSel_Alt0,   // b100
        FuncSel_Alt1,   // b101
        FuncSel_Alt2,   // b110
        FuncSel_Alt3,   // b111
    };

    typedef unsigned PinIdxType;

    static const std::size_t NumOfLines = 54;

    void configure(PinIdxType idx, FuncSel sel);
};

}  // namespace device


