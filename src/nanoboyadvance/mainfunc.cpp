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

const int g_width  = 240;
const int g_height = 160;

const int x_offset = (LV_HOR_RES - g_width)/2;
const int y_offset = (LV_VER_RES - g_height)/2;

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
    g_config.frameskip = 0;
    g_config.fast_forward = 10;

    //if (scale < 1) scale = 1;

    g_config.framebuffer = fbuffer;

    std::cout << "loading bios" << std::endl;
    g_emu.reloadConfig();

    std::cout << "loading rom" << std::endl;
    if (!File::exists(rom_path)) {
        std::cout << "ROM file not found." << std::endl;
        return -1;
    }
    if (!File::exists(g_config.bios_path)) {
        std::cout << "BIOS file not found." << std::endl;
        return -1;
    }

    std::cout << "loading game" << std::endl;
    auto cart = Cartridge::fromFile(rom_path);

    g_emu.loadGame(cart);
    keyinput = &g_emu.getKeypad();

    std::cout << "initializing window" << std::endl;
    setupWindow();

    std::cout << "starting emulation" << std::endl;

    while(true){
        updateInput();
        g_emu.runFrame();
        delay(3);
        drawFrame();
    }

    return 0;
}

void drawFrame(){
    static u32 scaled[LV_VER_RES * LV_HOR_RES]; 

    //nearest neighbor scaling
    const static int w1 = g_width, h1 = g_height, w2 = LV_HOR_RES, h2 = LV_VER_RES;
    const int x_ratio = ((w1<<16)/w2) + 1;
    const int y_ratio = ((h1<<16)/h2) + 1;
    for(int i = 0; i < h2; ++i){
        for(int j = 0; j < w2; ++j){
            int x2 = (j*x_ratio)>>16;
            int y2 = (i*y_ratio)>>16;
            scaled[(i*w2)+j] = fbuffer[(y2*w1)+x2];
        }
    }

    for(int i = 0; i < LV_VER_RES * LV_HOR_RES; ++i){
        framebuffer->buf[i].full = scaled[i];
        framebuffer->buf[i].alpha = 0;
    }
    lv_vdb_flush();

}

void setupWindow() {
    framebuffer = lv_vdb_get();
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
