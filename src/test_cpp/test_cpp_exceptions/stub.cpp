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


#include <cstdlib>

// This stub function is required by _mainCRTSturtup
extern "C" void exit(int status)
{
    static_cast<void>(status);
    while (true) {}
}

// This stub function is required by stdlib
extern "C" void _exit()
{
    exit(-1);
}

// This heap allocation stub is required by stdlib
extern "C" unsigned _sbrk(int inc)
{
    static_cast<void>(inc);
    return 0; // The application is not intended to be executed, no need for actual heap.
}

// This stub function is required by stdlib
extern "C" int _kill(int pid, int sig)
{
    static_cast<void>(pid);
    static_cast<void>(sig);
    return -1;
}

// This stub function is required by stdlib
extern "C" int _getpid(void) {
    return 1;
}

// This stub function is required by stdlib
extern "C" int _write(int file, char *ptr, int len)
{
    static_cast<void>(file);
    static_cast<void>(ptr);
    return len;
}

// This stub function is required by stdlib
extern "C" int _read(int file, char *ptr, int len)
{
    static_cast<void>(file);
    static_cast<void>(ptr);
    static_cast<void>(len);
    return 0;
}

// This stub function is required by stdlib
extern "C" int _close(int file)
{
    static_cast<void>(file);
    return -1;
}

// This stub function is required by stdlib
extern "C" int _fstat(int file, struct stat *st)
{
    static_cast<void>(file);
    static_cast<void>(st);
    return -1;
}

// This stub function is required by stdlib
extern "C" int _isatty(int file)
{
    static_cast<void>(file);
    return 1;
}

// This stub function is required by stdlib
extern "C" int _lseek(int file, int ptr, int dir)
{
    static_cast<void>(file);
    static_cast<void>(ptr);
    static_cast<void>(dir);
    return 0;
}

//namespace std
//{
//
//void __throw_out_of_range(char const*)
//{
//    while (true) {}
//}
//
//}
