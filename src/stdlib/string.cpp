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

#include <cstddef>

extern "C"
int memcmp( const void* lhs, const void* rhs, size_t count )
{
    auto lhsPtr = reinterpret_cast<const char*>(lhs);
    auto rhsPtr = reinterpret_cast<const char*>(rhs);
    int diffSum = 0;
    while (0 < count) {
        diffSum += static_cast<int>(*lhsPtr) - static_cast<int>(*rhsPtr);
        if (diffSum != 0) {
            break;
        }
        --count;
        ++lhsPtr;
        ++rhsPtr;
    }
    return diffSum;
}

extern "C"
void* memset(void* dest, int ch, size_t count)
{
    auto destPtr = reinterpret_cast<unsigned char*>(dest);
    auto castedCh = static_cast<unsigned char>(ch);
    while (0 < count) {
        *destPtr = castedCh;
        ++destPtr;
        --count;
    }

    return dest;
}
