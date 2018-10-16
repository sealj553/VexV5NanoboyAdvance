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

#include <string>
#include <iostream>

#include "core/system/gba/emulator.hpp"
#include "util/file.hpp"
#include "util/ini.hpp"

#include "version.hpp"

#include "api.h"

using namespace std;
using namespace Util;
using namespace Core;
using namespace pros::c;

int g_width  = 240;
int g_height = 160;

lv_vdb_t *framebuffer;
u16* keyinput;
u32 fbuffer[240 * 160];

Config   g_config;
Emulator g_emu(&g_config);
INI*     g_ini;

const std::string g_version_title = "NanoboyAdvance " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR);

void setupWindow();
void drawFrame();
void updateInput();

int start_emulator() {
    //int scale = 1;
    //TODO: make game chooser
    std::string rom_path = "/usd/game.gba";

    g_ini = new INI("/usd/config.ini");

    // [Emulation]
    g_config.bios_path  = "/usd/bios.bin";
    g_config.multiplier = g_ini->getInteger("Emulation", "fastforward");

    // [Video]
    //scale                  = g_ini->getInteger("Video", "scale");
    //g_config.darken_screen = g_ini->getInteger("Video", "darken"); // boolean!
    g_config.frameskip     = g_ini->getInteger("Video", "frameskip");

    //if (scale < 1) scale = 1;

    g_config.framebuffer = fbuffer;

    g_emu.reloadConfig();

    if (!File::exists(rom_path)) {
        std::cout << "ROM file not found." << std::endl;
        return -1;
    }
    if (!File::exists(g_config.bios_path)) {
        std::cout << "BIOS file not found." << std::endl;
        return -1;
    }

    auto cart = Cartridge::fromFile(rom_path);

    g_emu.loadGame(cart);
    keyinput = &g_emu.getKeypad();

    // setup window
    //g_width  *= scale;
    //g_height *= scale;
    setupWindow();

    bool running = true;

    int frames = 0;
    int ticks1 = millis();

    while (running) {
        // generate frame(s)
        g_emu.runFrame();

        // update frame counter
        frames += g_config.fast_forward ? g_config.multiplier : 1;

        int ticks2 = millis();
        if (ticks2 - ticks1 >= 1000) {
            int percentage = (frames / 60.0) * 100;
            int rendered_frames = frames;

            if (g_config.fast_forward) {
                rendered_frames /= g_config.multiplier;
            }

            ticks1 = ticks2;
            frames = 0;
        }

        drawFrame();
        updateInput();
    }

    std::cout << "Quitting..." << std::endl;

    return 0;
}

void drawFrame(){

#define ARGB8888_R(color) (uint8_t((color & 0x00FF0000) >> 16))
#define ARGB8888_G(color) (uint8_t((color & 0x0000FF00) >> 8))
#define ARGB8888_B(color) (uint8_t((color & 0x000000FF)))
#define ARGB8888_A(color) (uint8_t((color & 0xFF000000) >> 24))

    for(int y = 0; y < g_height; ++y){
        for(int x = 0; x < g_width; ++x){
            u32 col = fbuffer[y * g_width + x];

            lv_color_t color = { 
                ARGB8888_B(col),
                ARGB8888_G(col),
                ARGB8888_R(col),
                0
            };

            //TODO: add center offset and maybe scale
            (*framebuffer).buf[y * LV_HOR_RES + x] = color;
        }
    }

    lv_vdb_flush();
}

void setupWindow() {
    framebuffer = lv_vdb_get();
    memset((*framebuffer).buf, 0, LV_HOR_RES * LV_VER_RES * sizeof(lv_color_t));
}

void updateInput(){
    using namespace pros;

    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))     (*keyinput) = (1<<0); //a 
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))     (*keyinput) = (1<<1); //b
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y))     (*keyinput) = (1<<2); //select
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))     (*keyinput) = (1<<3); //start
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT)) (*keyinput) = (1<<4); //right
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT))  (*keyinput) = (1<<5); //left
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP))    (*keyinput) = (1<<6); //up
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_DOWN))  (*keyinput) = (1<<7); //down
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))    (*keyinput) = (1<<8); //rb
    if(controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1))    (*keyinput) = (1<<9); //lb
}
