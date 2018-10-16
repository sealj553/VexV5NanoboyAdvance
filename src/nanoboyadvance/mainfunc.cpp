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

#include "mainfunc.hpp"
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
    //todo make game chooser
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
    // send generated frame to texture
    //SDL_UpdateTexture(g_texture, NULL, fbuffer, 240 * sizeof(u32));
    //SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);

}

void setupWindow() {
    framebuffer = lv_vdb_get();
}


void updateInput(){

    using namespace pros;
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_DOWN);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y);
    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A);


/*
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            int  num;
            bool released = event.type == SDL_KEYUP;

            SDL_KeyboardEvent* key_event = (SDL_KeyboardEvent*)(&event);

            switch (key_event->keysym.sym) {
                case SDLK_z:
                case SDLK_y:         num = (1<<0); break;
                case SDLK_x:         num = (1<<1); break;
                case SDLK_BACKSPACE: num = (1<<2); break;
                case SDLK_RETURN:    num = (1<<3); break;
                case SDLK_RIGHT:     num = (1<<4); break;
                case SDLK_LEFT:      num = (1<<5); break;
                case SDLK_UP:        num = (1<<6); break;
                case SDLK_DOWN:      num = (1<<7); break;
                case SDLK_w:         num = (1<<8); break;
                case SDLK_q:         num = (1<<9); break;
                case SDLK_SPACE:
                                     if (released) {
                                         g_config.fast_forward = false;
                                     } else {
                                         // prevent more than one update!
                                         if (g_config.fast_forward) {
                                             continue;
                                         }
                                         g_config.fast_forward = true;
                                     }
                                     continue;
                case SDLK_F9:
                                     g_emu.reset();
                                     continue;
                default:
                                     continue;
            }

            if (released) {
                *keyinput |= num;
            } else {
                *keyinput &= ~num;
            }
        }
    }
*/

}
