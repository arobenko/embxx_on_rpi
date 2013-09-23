//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//


// For some reason in place new also requires delete.
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



