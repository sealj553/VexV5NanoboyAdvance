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

const std::string g_version_title = "NanoboyAdvance " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR);

void setupWindow();
void drawFrame();
void updateInput();

int start_emulator() {
    //int scale = 1;
    //TODO: make game chooser
    std::string rom_path = "/usd/game.gba";

    // [Emulation]
    g_config.bios_path  = "/usd/bios.bin";
    g_config.multiplier = 1;

    // [Video]
    //scale                  = 1;
    //g_config.darken_screen = 0;
    g_config.frameskip = 3;
    g_config.fast_forward = 0;

    //if (scale < 1) scale = 1;

    g_config.framebuffer = fbuffer;

    g_emu.reloadConfig();

    std::cout << "checking if rom exists" << std::endl;
    if (!File::exists(rom_path)) {
        std::cout << "ROM file not found." << std::endl;
        return -1;
    }
    if (!File::exists(g_config.bios_path)) {
        std::cout << "BIOS file not found." << std::endl;
        return -1;
    }
    std::cout << "done" << std::endl;

    auto cart = Cartridge::fromFile(rom_path);

    g_emu.loadGame(cart);
    std::cout << "game loaded" << std::endl;
    keyinput = &g_emu.getKeypad();

    // setup window
    //g_width  *= scale;
    //g_height *= scale;
    setupWindow();
    std::cout << "window initialized" << std::endl;

    bool running = true;

    int frames = 0;
    int oldTime = millis();

    while (running) {
        // generate frame(s)
        g_emu.runFrame();

        /*
        // update frame counter
        frames += g_config.fast_forward ? g_config.multiplier : 1;

        int time = millis();
        if (time - oldTime >= 1000) {
            //int percentage = (frames / 60.0) * 100;
            int rendered_frames = frames;

            if (g_config.fast_forward) {
                rendered_frames /= g_config.multiplier;
            }

            oldTime = time;
            frames = 0;
        }
        */

        drawFrame();
        delay(3);
        updateInput();
    }

    std::cout << "Quitting..." << std::endl;

    return 0;
}

void drawFrame(){

#define ARGB8888_R(color) (uint8_t((color & 0x00FF0000) >> 16))
#define ARGB8888_G(color) (uint8_t((color & 0x0000FF00) >> 8))
#define ARGB8888_B(color) (uint8_t((color & 0x000000FF)))
//#define ARGB8888_A(color) (uint8_t((color & 0xFF000000) >> 24))

    lv_color_t color;

    for(int y = 0; y < g_height; ++y){
        for(int x = 0; x < g_width; ++x){
            u32 col = fbuffer[y * g_width + x];

            color.blue = ARGB8888_B(col);
            color.green = ARGB8888_G(col);
            color.red = ARGB8888_R(col);

            //TODO: add center offset and maybe scale
            framebuffer->buf[y * LV_HOR_RES + x] = color;

            //printf("\n");
            //std::cout << std::endl;
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
    using namespace Core;

    g_emu.setKeyState(Key::A,       controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A));
    g_emu.setKeyState(Key::B,       controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B));
    g_emu.setKeyState(Key::Start,   controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X));
    g_emu.setKeyState(Key::Select,  controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y));
    g_emu.setKeyState(Key::Right,   controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT));
    g_emu.setKeyState(Key::Left,    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT));
    g_emu.setKeyState(Key::Up,      controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP));
    g_emu.setKeyState(Key::Down,    controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_DOWN));
    g_emu.setKeyState(Key::R,       controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1));
    g_emu.setKeyState(Key::L,       controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1));
}
