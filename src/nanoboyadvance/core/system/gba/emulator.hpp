/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
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

#include <string>

#include "enums.hpp"
#include "config.hpp"
#include "interrupt.hpp"
#include "dma/regs.hpp"
#include "timer/regs.hpp"
#include "ppu/ppu.hpp"
#include "apu/apu.hpp"
#include "cart/cartridge.hpp"

#include "gpio.hpp"
#include "rtc.hpp"

#include "../../processor/arm/arm.hpp"

namespace Core {

    class Emulator : private ARM {
    public:
        Emulator(Config* config);
        ~Emulator();

        void reset();

        APU& getAPU();
        u16& getKeypad();
        void setKeyState(Key key, bool pressed);

        void reloadConfig();
        void loadGame(std::shared_ptr<Cartridge> cart);

        void runFrame();

    private:
        Config* config;

        GPIO* gpio = new RTC(regs.irq.flag);

        // Cycles until next PPU phase
        int cycles_left;

        // Cycle count LUTs
        int cycles  [2][16];
        int cycles32[2][16];

        // Cycle count configurations for different waitstates
        static constexpr int s_ws_nseq[4] = { 4, 3, 2, 8 }; // non-sequential SRAM/WS0/WS1/WS2
        static constexpr int s_ws_seq0[2] = { 2, 1 };       // sequential WS0
        static constexpr int s_ws_seq1[2] = { 4, 1 };       // sequential WS1
        static constexpr int s_ws_seq2[2] = { 8, 1 };       // sequential WS2

        // DMA mask LUTs
        static constexpr u32 s_dma_dst_mask[4] = { 0x07FFFFFF, 0x07FFFFFF, 0x07FFFFFF, 0x0FFFFFFF };
        static constexpr u32 s_dma_src_mask[4] = { 0x07FFFFFF, 0x0FFFFFFF, 0x0FFFFFFF, 0x0FFFFFFF };
        static constexpr u32 s_dma_len_mask[4] = { 0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF };

        // Subsystems (PPU, APU, shitty interrupt thing)
        PPU ppu;
        APU apu;
        Interrupt m_interrupt;

        // Do not delete - needed for reference counting
        std::shared_ptr<Cartridge> cart;

        struct SystemMemory {
            u8 bios    [0x04000];
            u8 wram    [0x40000];
            u8 iram    [0x08000];
            u8 palette [0x00400];
            u8 oam     [0x00400];
            u8 vram    [0x18000];

            // Local copy (fast access)
            struct ROM {
                u8*    data;
                Save*  save;
                size_t size;
            } rom;

            u32 bios_opcode;

            //TODO: remove this hack
            u8 mmio[0x800];
        } memory;

        struct Registers {
            DMA   dma[4];
            Timer timer[4];

            u16 keyinput;

            struct Interrupt {
                u16 enable;
                u16 flag;
                u16 master_enable;
            } irq;

            struct WaitstateControl {
                int sram;
                int ws0_n;
                int ws0_s;
                int ws1_n;
                int ws1_s;
                int ws2_n;
                int ws2_s;
                int phi;
                int prefetch;
                int cgb;
            } waitcnt;

            SystemState haltcnt;
        } regs;

        // Memory Mapped I/O
        auto readMMIO (u32 address) -> u8;
        void writeMMIO(u32 address, u8 value);

        void runInternal(int cycles);

        // DMA emulation
        int  dma_running;
        int  dma_current;
        bool dma_loop_exit;

        // DMA I/O reset/read/write
        void dmaReset(int id);
        auto dmaRead (int id, int offset) -> u8;
        void dmaWrite(int id, int offset, u8 value);

        void dmaActivate(int id);

        // DMA handling
        void dmaFindHBlank();
        void dmaFindVBlank();
        void dmaTransfer();
        void dmaTransferFIFO(int dma_id);

        // Timer I/O reset/read/write
        void timerReset(int id);
        auto timerRead (int id, int offset) -> u8;
        void timerWrite(int id, int offset, u8 value);

        // Timer handling
        void timerStep(int cycles);
        
        template <int id>
        void timerRunInternal(int cycles);
        void timerHandleFIFO(int timer_id, int times);

        void calculateMemoryCycles();
    protected:
        // Memory bus implementation
        #include "memory/memory.hpp"

        void busInternalCycles(int count) {
            cycles_left -= count;
        }

        void handleSWI(int number) final {};
    };
}
