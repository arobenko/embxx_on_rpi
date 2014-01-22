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

#include <functional>


namespace std
{

namespace placeholders
{

decltype(std::placeholders::_1) _1;
decltype(std::placeholders::_2) _2;
decltype(std::placeholders::_3) _3;
decltype(std::placeholders::_4) _4;

}  // namespace placeholders

}  // namespace std


