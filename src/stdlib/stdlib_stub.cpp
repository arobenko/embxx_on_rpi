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


// For some reason placement new also requires delete.
void operator delete(void*)
{
}

// Compiler requires this function when there are pure virtual functions
extern "C" void __cxa_pure_virtual()
{
}

// Compiler requires this function when there are static area objects that
// require custom destructors
extern "C" void __aeabi_atexit()
{
}

// Compiler requires this symbol when there are static area objects that
// require custom destructors
void* __dso_handle = 0;



