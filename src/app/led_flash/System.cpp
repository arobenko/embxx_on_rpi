#include "System.h"

System& System::instance()
{
    static System system;
    return system;
}

System::System()
    : gpio_(func_),
      led_(gpio_)
{
}

extern "C"
void interruptHandler()
{
}
