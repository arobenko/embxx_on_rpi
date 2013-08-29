//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#pragma once

#include "Function.h"

namespace device
{

class Gpio
{
public:
    typedef Function::PinIdxType PinIdxType;

    enum Dir {
        Dir_Input,
        Dir_Output,
        Dir_NumOfDirs // Must be last
    };

    explicit Gpio(Function& func);

    void configDir(PinIdxType idx, Dir dir);

    void writePin(PinIdxType idx, bool value);

    bool readPin(PinIdxType idx) const;

private:
    Function& func_;
};

}  // namespace device


