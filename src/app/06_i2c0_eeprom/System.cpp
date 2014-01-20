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

#include "System.h"

System& System::instance()
{
    static System system;
    return system;
}

System::System()
    : gpio_(interruptMgr_, func_),
      uart_(interruptMgr_, func_, SysClockFreq),
      i2c_(interruptMgr_, func_),
      i2cOpQueue_(i2c_),
      i2cCharAdapter_(i2cOpQueue_, EepromAddress),
      uartDriver_(uart_, el_),
      i2cDriver_(i2cCharAdapter_, el_),
      led_(gpio_),
      eeprom_(i2cDriver_),
      buf_(uartDriver_),
      stream_(buf_),
      log_("\r\n", stream_)
{
    uart_.configBaud(115200);
    uart_.setWriteEnabled(true);
    i2c_.setDivider(SysClockFreq/I2cFreq);
}

extern "C"
void interruptHandler()
{
    System::instance().interruptMgr().handleInterrupt();
}
