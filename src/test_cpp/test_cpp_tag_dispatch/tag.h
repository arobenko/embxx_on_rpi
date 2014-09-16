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

struct Tag1 {};
struct Tag2 {};

class Dispatcher
{
public:

    template <typename TTag>
    static void func()
    {
        funcInternal(TTag());
    }

    template <typename TTag>
    static void otherFunc()
    {
        otherFuncInternal(TTag());
    }

private:
    static void funcInternal(Tag1 tag);
    static void funcInternal(Tag2 tag);

    static void otherFuncInternal(Tag1 tag)
    {
        static_cast<void>(tag);
        otherFuncTag1();
    }

    static void otherFuncInternal(Tag2 tag)
    {
        static_cast<void>(tag);
        otherFuncTag2();
    }


    static void otherFuncTag1();
    static void otherFuncTag2();
};
