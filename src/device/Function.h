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
    enum class FuncSel {
        Input,  // b000
        Output, // b001
        Alt5,   // b010
        Alt4,   // b011
        Alt0,   // b100
        Alt1,   // b101
        Alt2,   // b110
        Alt3,   // b111
    };

    typedef unsigned PinIdxType;

    static const std::size_t NumOfLines = 54;

    void configure(PinIdxType idx, FuncSel sel);
};

}  // namespace device


