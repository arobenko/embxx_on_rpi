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
      spi_(interruptMgr_, func_),
      spiOpQueue_(spi_),
      spiCharAdapter_(spiOpQueue_, SpiDevIdx),
      uartDriver_(uart_, el_),
      spiDriver_(spiCharAdapter_, el_),
      led_(gpio_),
      buf_(uartDriver_),
      stream_(buf_),
      log_("\r\n", stream_),
      spiInBuf_(spiDriver_)
{
    uart_.configBaud(115200);
    uart_.setWriteEnabled(true);
    spi_.setFreq(SysClockFreq, InitialSpiFreq);
    spi_.setMode(Spi::Mode0);
}

extern "C"
void interruptHandler()
{
    System::instance().interruptMgr().handleInterrupt();
}
