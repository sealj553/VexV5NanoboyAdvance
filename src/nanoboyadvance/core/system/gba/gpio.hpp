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

#pragma once

#include <cstdint>

#include "enums.hpp"

namespace Core {

    class GPIO {
    protected:
        enum IOPortDirection {
            GPIO_DIR_IN  = 0, // GPIO -> GBA
            GPIO_DIR_OUT = 1  // GPIO <- GBA
        };

    private:
        enum IOPort {
            GPIO_DATA       = 0xC4,
            GPIO_DIRECTION  = 0xC6,
            GPIO_CONTROL    = 0xC8
        };

        bool allow_reads { false };

        IOPortDirection port_dir[4] {
            GPIO_DIR_IN, GPIO_DIR_IN, 
            GPIO_DIR_IN, GPIO_DIR_IN
        };

        std::uint16_t& irq_flags;

        std::uint8_t read_mask  { 0 };
        std::uint8_t write_mask { 15 };

        std::uint8_t port_data { 0 };
    public:
        GPIO(std::uint16_t& irq_flags) : irq_flags(irq_flags) { }

        virtual void reset() {
            // TODO: verify these settings
            this->port_dir[0] = GPIO_DIR_OUT;
            this->port_dir[1] = GPIO_DIR_OUT;
            this->port_dir[2] = GPIO_DIR_OUT;
            this->port_dir[3] = GPIO_DIR_OUT;

            this->port_data = 0;

            updateReadWriteMasks();
        }

        void updateReadWriteMasks();

        auto read(std::uint32_t address) -> std::uint8_t;

        void write(std::uint32_t address, std::uint8_t value);

        auto isReadable() -> bool { return this->allow_reads; }

    protected:
        void sendIRQ() {
            irq_flags |= INTERRUPT_GAMEPAK;
        }

        auto portDirection(int port) -> IOPortDirection const {
            if (port >= 4) {
                return GPIO_DIR_OUT;
            }
            return this->port_dir[port];
        }

        virtual auto readPort() -> std::uint8_t { return 0; }

        virtual void writePort(std::uint8_t data) {}
    };
}