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

#include "rtc.hpp"
#include "util/logger.hpp"

using namespace Util;

namespace Core {
    constexpr int RTC::s_num_params[8];

    auto RTC::readSIO() -> bool {
        this->byte_reg &= ~(1 << this->idx_bit);
        this->byte_reg |=  (this->port.sio << this->idx_bit);

        if (++this->idx_bit == 8) {
            //Logger::log<LOG_DEBUG>("RTC: byte_reg=0x{0:X}", this->byte_reg);
            this->idx_bit  = 0;
            return true;
        }

        return false;
    }

    auto RTC::readPort() -> std::uint8_t {
        if (this->state == SENDING) {
            //Logger::log<LOG_DEBUG>("RTC: read={0}", this->port.sio);
            return this->port.sio << PORT_SIO;
        }
        // Ugh tri-state logic, "High-Z". idk.
        return 1;
    }

    void RTC::writePort(std::uint8_t data) {
        // Verify port directions.
        if (portDirection(PORT_CS) != GPIO::GPIO_DIR_OUT) {
            Logger::log<LOG_WARN>("RTC: wrong CS port direction.");
        }
        if (portDirection(PORT_SCK) != GPIO::GPIO_DIR_OUT) {
            Logger::log<LOG_WARN>("RTC: wrong SCK port direction.");
        }

        int old_sck = this->port.sck;
        int old_cs  = this->port.cs;

        int sck = (data>>PORT_SCK)&1;
        int sio = (data>>PORT_SIO)&1;
        int cs  = (data>>PORT_CS )&1;

        // Update port status accordingly
        if (portDirection(PORT_SCK) == GPIO::GPIO_DIR_OUT) this->port.sck = sck;
        if (portDirection(PORT_SIO) == GPIO::GPIO_DIR_OUT) this->port.sio = sio;
        if (portDirection(PORT_CS ) == GPIO::GPIO_DIR_OUT) this->port.cs  = cs;

        //Logger::log<LOG_DEBUG>("RTC: sck={0} sio={1} cs={2}", this->port.sck, this->port.sio, this->port.cs);

        if (!old_cs &&  cs) {
            Logger::log<LOG_DEBUG>("RTC: enabled.");

            // Not tested but probably needed.
            this->state    = WAIT_CMD;
            this->idx_bit  = 0;
            this->idx_byte = 0;
        }
        if ( old_cs && !cs) Logger::log<LOG_DEBUG>("RTC: disabled.");

        if (!cs) return;

        switch (this->state) {
            case WAIT_CMD: {
                // CHECKME: seems like data should be accepted on rising clock edge.
                if (!old_sck && sck) {
                    processCommandBit();
                }
                break;
            }
            case SENDING: {
                // CHECKME: seems like data should be output on falling clock edge.
                if (old_sck && !sck) {
                    this->port.sio = this->data[this->idx_byte] & 1;
                    this->data[this->idx_byte] >>= 1;

                    if (++this->idx_bit == 8) {
                        this->idx_bit = 0;
                        if (++this->idx_byte >= s_num_params[this->cmd]) {
                            // TODO: check docs, if chip really accepts another command.
                            this->state = WAIT_CMD;
                        }
                    }
                }
                break;
            }
            case RECEIVING: {
                // CHECKME: seems like data should be accepted on rising clock edge.
                if (!old_sck && sck) {
                    // Receive bytes until all required params have been received.
                    if (this->idx_byte < s_num_params[this->cmd]) {
                        if (readSIO()) {
                            //Logger::log<LOG_DEBUG>("RTC: received byte {0}", this->idx_byte);
                            this->data[this->idx_byte++] = this->byte_reg;
                        }
                    }
                    // Need to recheck if all bytes have been received, as idx_byte is modified above.
                    if (this->idx_byte == s_num_params[this->cmd]) {
                        // In that case finally dispatch the command.
                        Logger::log<LOG_DEBUG>(
                            "RTC: cmd_buf(in)=[0x{0:X}, 0x{1:X}, 0x{2:X}, 0x{3:X}, 0x{4:X}, 0x{5:X}, 0x{6:X}]",
                            this->data[0], this->data[1], this->data[2], this->data[3], 
                            this->data[4], this->data[5], this->data[6]
                        );
                        writeRTC(static_cast<RTCRegister>(this->cmd));
                        
                        // TODO: check docs, if chip really accepts another command.
                        this->state = WAIT_CMD;
                    }
                }
                break;
            }
        }
    }

    void RTC::processCommandBit() {
        bool completed = readSIO();

        // Wait until the complete CMD byte arrived.
        if (!completed) return;

        uint8_t cmd = 0;

        // Check for FWD/REV format specifier
        if ((this->byte_reg & 15) == 6) {
            cmd = this->byte_reg;
        }
        else if ((this->byte_reg >> 4) == 6) {
            Logger::log<LOG_DEBUG>("RTC: game uses REV format.");
            // Reverse bit pattern.
            for (int i = 0; i < 7; i++)
                cmd |= ((this->byte_reg>>(i^7))&1)<<i;
        }
        else {
            Logger::log<LOG_WARN>("RTC: undefined state: unknown command format. byte=0x{0:X}", this->byte_reg);
            return;
        }

        // Extract RTC register ("cmd") and reset receive indices
        this->cmd = (cmd>>4)&7;
        this->idx_byte = 0;
        this->idx_bit  = 0;

        // cmd[7:] determines if the RTC register is read or written.
        if (cmd & 0x80) {
            Logger::log<LOG_DEBUG>("RTC: cmd={0} (read)", this->cmd);

            readRTC(static_cast<RTCRegister>(this->cmd));
            
            if (s_num_params[this->cmd] > 0) {
                this->state = SENDING;
            }
            else {
                this->state = WAIT_CMD;
            }
        }
        else {
            Logger::log<LOG_DEBUG>("RTC: cmd={0} (write)", this->cmd);
            
            if (s_num_params[this->cmd] > 0) {
                this->state = RECEIVING;
            }
            else {
                writeRTC(static_cast<RTCRegister>(this->cmd));
                this->state = WAIT_CMD;
            }
        }
    }

    void RTC::readRTC(RTCRegister reg) {
        switch (reg) {
            case CONTROL: {
                this->data[0] = (this->control.unknown    ? 2   : 0) |
                                (this->control.minute_irq ? 8   : 0) |
                                (this->control.mode_24h   ? 64  : 0) |
                                (this->control.power_off  ? 128 : 0);
                Logger::log<LOG_DEBUG>("RTC: read: control=[0x{0:X}]", this->data[0]);
                break;
            }
            case DATETIME: {
                this->data[0] = datetime.year;
                this->data[1] = datetime.month;
                this->data[2] = datetime.day;
                this->data[3] = datetime.day_of_week;
                this->data[4] = datetime.hour;
                this->data[5] = datetime.minute;
                this->data[6] = datetime.second;
                Logger::log<LOG_DEBUG>(
                    "RTC: read: datetime=[0x{0:X}, 0x{1:X}, 0x{2:X}, 0x{3:X}, 0x{4:X}, 0x{5:X}, 0x{6:X}]",
                    this->data[0], this->data[1], this->data[2], this->data[3], 
                    this->data[4], this->data[5], this->data[6]
                );
                break;
            }
            case TIME: {
                this->data[0] = datetime.hour;
                this->data[1] = datetime.minute;
                this->data[2] = datetime.second;
                Logger::log<LOG_DEBUG>(
                    "RTC: read: time=[0x{0:X}, 0x{1:X}, 0x{2:X}]",
                    this->data[0], this->data[1], this->data[2]
                );
                break;
            }
            default: {
                Logger::log<LOG_DEBUG>("RTC: read: unhandled register=0x{0:X}", reg);
            }
        }
    }

    void RTC::writeRTC(RTCRegister reg) {
        switch (reg) {
            case CONTROL: {
                auto value = this->data[0];

                this->control.unknown    = value & 2;
                this->control.minute_irq = value & 8 ;
                this->control.mode_24h   = value & 64;

                Logger::log<LOG_DEBUG>("RTC: write: control=[0x{0:X}]", value);

                if (this->control.minute_irq) Logger::log<LOG_WARN>("RTC: write: minute IRQ enabled!!");

                break;
            }
            case FORCE_RESET: {
                this->control.reset();
                this->datetime.reset();

                Logger::log<LOG_DEBUG>("RTC: write: forced reset");
                break;
            }
            case FORCE_IRQ: {
                sendIRQ();

                Logger::log<LOG_DEBUG>("RTC: write: forced IRQ");
                break;
            }
            default: {
                Logger::log<LOG_DEBUG>("RTC: write: unhandled register=0x{0:X}", reg);
            }
        }
    }
}
