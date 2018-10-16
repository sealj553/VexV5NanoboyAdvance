/**
  * Copyright (C) 2018 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include "gpio.hpp"

#include "util/logger.hpp"

using namespace Util;

namespace Core {
    void GPIO::updateReadWriteMasks() {
        this->read_mask = 0;

        if (portDirection(0) == GPIO_DIR_IN) this->read_mask |= 1;
        if (portDirection(1) == GPIO_DIR_IN) this->read_mask |= 2;
        if (portDirection(2) == GPIO_DIR_IN) this->read_mask |= 4;
        if (portDirection(3) == GPIO_DIR_IN) this->read_mask |= 8;
    
        this->write_mask = ~(this->read_mask)&15;

        Logger::log<LOG_DEBUG>("GPIO: read_mask=0b{0:B} write_mask=0b{1:B}", this->read_mask, this->write_mask);
    }

    auto GPIO::read(std::uint32_t address) -> std::uint8_t {
        Logger::log<LOG_DEBUG>("GPIO: read from 0x{0:X}", address);

        if (!this->allow_reads) return 0;

        switch (address) {
            case GPIO_DATA: {
                auto value = readPort() & this->read_mask;

                this->port_data &= ~(this->read_mask);
                this->port_data |= value;
                
                // CHECKME: I suspect writable bits are low or floating when being read but
                // are not actually readable? Currently we just return 0 for those bits.
                return value;
            }
            case GPIO_DIRECTION: {
                return this->read_mask;
            }
            case GPIO_CONTROL: {
                return this->allow_reads ? 1 : 0;
            }
        }

        return 0;
    }

    void GPIO::write(std::uint32_t address, std::uint8_t value) {
        Logger::log<LOG_DEBUG>("GPIO: write to 0x{0:X} = 0b{1:B}", address, value);

        switch (address) {
            case GPIO_DATA: {
                this->port_data &= ~(this->write_mask);
                this->port_data |=   this->write_mask & value;
                writePort(this->port_data);
                break;
            }
            case GPIO_DIRECTION: {
                // TODO: interdpth RTC seems to setup SIO port direction up incorrectly.
                this->port_dir[0] = static_cast<IOPortDirection>((value>>0)&1);
                this->port_dir[1] = static_cast<IOPortDirection>((value>>1)&1);
                this->port_dir[2] = static_cast<IOPortDirection>((value>>2)&1);
                this->port_dir[3] = static_cast<IOPortDirection>((value>>3)&1);

                updateReadWriteMasks();

                Logger::log<LOG_DEBUG>("GPIO: port_dir[0]={0}", this->port_dir[0]);
                Logger::log<LOG_DEBUG>("GPIO: port_dir[1]={0}", this->port_dir[1]);
                Logger::log<LOG_DEBUG>("GPIO: port_dir[2]={0}", this->port_dir[2]);
                Logger::log<LOG_DEBUG>("GPIO: port_dir[3]={0}", this->port_dir[3]);
                break;
            }
            case GPIO_CONTROL: {
                this->allow_reads = value & 1;
                if (this->allow_reads) {
                    Logger::log<LOG_DEBUG>("GPIO: enabled reading");
                }
                else {
                    Logger::log<LOG_DEBUG>("GPIO: disabled reading");
                }
                break;
            }
        }
    }
}