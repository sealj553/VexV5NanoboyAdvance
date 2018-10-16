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

int g_width  = 240;
int g_height = 160;

lv_vdb_t *framebuffer;
u16* keyinput;

struct controller_state {
    int32_t
        a_lx,
        a_ly,
        a_rx,
        a_ry,
        d_l1,
        d_l2,
        d_r1,
        d_r2,
        d_up,
        d_dwn,
        d_lft,
        d_rig,
        d_x,
        d_b,
        d_y,
        d_a;
}; 

static struct controller_state c_state = { 0 };
static struct controller_state c_oldstate = { 0 };

Config   g_config;
Emulator g_emu(&g_config);
INI*     g_ini;

const std::string g_version_title = "NanoboyAdvance " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR);

void setupWindow();
void drawFrame();
void updateController();
void updateInput();

int start_emulator() {
    u32  fbuffer[240 * 160];

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
    while(1){}

    return 0;
}

void drawFrame(){
        // send generated frame to texture
        //SDL_UpdateTexture(g_texture, NULL, fbuffer, 240 * sizeof(u32));

        //// tell SDL to draw the texture
        //SDL_RenderClear(g_renderer);
        //SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
        //SDL_RenderPresent(g_renderer);

}

void setupWindow() {
    framebuffer = lv_vdb_get();

    //SDL_CreateWindow(g_version_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_width, g_height, 0);
    //SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    //SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
}

void updateController(){
    c_state.a_lx  = controller_get_analog(E_CONTROLLER_MASTER,  E_CONTROLLER_ANALOG_LEFT_X);
    c_state.a_ly  = controller_get_analog(E_CONTROLLER_MASTER,  E_CONTROLLER_ANALOG_LEFT_Y);
    c_state.a_rx  = controller_get_analog(E_CONTROLLER_MASTER,  E_CONTROLLER_ANALOG_RIGHT_X);
    c_state.a_ry  = controller_get_analog(E_CONTROLLER_MASTER,  E_CONTROLLER_ANALOG_RIGHT_Y);
    c_state.d_l1  = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1);
    c_state.d_l2  = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2);
    c_state.d_r1  = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1);
    c_state.d_r2  = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2);
    c_state.d_up  = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP);
    c_state.d_dwn = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_DOWN);
    c_state.d_lft = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT);
    c_state.d_rig = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT);
    c_state.d_x   = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X);
    c_state.d_b   = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B);
    c_state.d_y   = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y);
    c_state.d_a   = controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A);
}

void updateInput(){

        c_oldstate = c_state;
        updateController();
        


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


}
