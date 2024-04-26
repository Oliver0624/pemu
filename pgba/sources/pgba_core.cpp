//
// Created by Oliver on 04/25/2024
//

#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "c2dui.h"
#include "uiEmu.h"

#include "libretro.h"
#include "pgba_core.h"

extern PGBAUiEmu *uiEmu;

typedef uint16_t color_t;
#define BYTES_PER_PIXEL 2


#define MGBA_WIDTH_PIXEL       256
#define MGBA_HEIGHT_PIXEL      224

#define SIZE_CART_FLASH1M    0x00020000

static unsigned _width  = MGBA_WIDTH_PIXEL;
static unsigned _height = MGBA_HEIGHT_PIXEL; 
static char _savedata[SIZE_CART_FLASH1M];
static std::string _savePath;
static bool _running = false;

void pgba_callback_printf(enum retro_log_level level, const char *fmt, ...) {
    if (level <= RETRO_LOG_INFO) {
        return;
    }
    char szLogMsg[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(szLogMsg, sizeof(szLogMsg), fmt, ap);
    va_end(ap);
    printf("[MGBA] [%d] %s", (int)level, szLogMsg);
    return;
}

bool pgba_callback_env(unsigned cmd, void *data) {
    switch(cmd) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
        struct retro_log_callback *log = (struct retro_log_callback *)data;
        log->log = pgba_callback_printf;
        break;
    }
    }
    return true;
}

void pgba_callback_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (width != _width || height != _height) {
        color_t* outputBuffer;
        int stride = 0;
        uiEmu->addVideo((uint8_t**)&outputBuffer, &stride, {width, height}, {width, height});
        retro_set_video_buffer(outputBuffer, width);
        _width = width;
        _height = height;
    }
    return;
}

size_t pgba_callback_audio_sample_batch(const int16_t *data, size_t frames) {
    uiEmu->getAudio()->play(data, frames, Audio::SyncMode::None);
    return 0;
}

void pgba_callback_input_poll(void) {
    return;
}

int16_t pgba_callback_input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (RETRO_DEVICE_ID_JOYPAD_MASK != id) {
        return 0;
    }

    uint16_t keys = 0;
    auto ui = uiEmu->getUi();
    auto *players = ui->getInput()->getPlayers();
    auto buttons = players[0].buttons;
    if (buttons) {
        if (buttons & c2d::Input::Button::Up)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_UP;
        if (buttons & c2d::Input::Button::Down)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_DOWN;
        if (buttons & c2d::Input::Button::Left)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_LEFT;
        if (buttons & c2d::Input::Button::Right)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_RIGHT;
        if (buttons & c2d::Input::Button::A)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_A;
        if (buttons & c2d::Input::Button::B)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_B;
        if (buttons & c2d::Input::Button::LT)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_L;
        if (buttons & c2d::Input::Button::RT)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_R;
        if (buttons & c2d::Input::Button::Select)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_SELECT;
        if (buttons & c2d::Input::Button::Start)
            keys |= 1 << RETRO_DEVICE_ID_JOYPAD_START;
    }
    return keys;
}

int mgba_init() {
    retro_set_environment(pgba_callback_env);
    retro_set_video_refresh(pgba_callback_video_refresh);
    retro_set_audio_sample_batch(pgba_callback_audio_sample_batch);
    retro_set_input_poll(pgba_callback_input_poll);
    retro_set_input_state(pgba_callback_input_state);
    retro_init();

    retro_set_savedate((void*)_savedata);
    return 0;
}

void mgba_fini() {
    retro_deinit();
}

int mgba_start(const ss_api::Game &game) {
    color_t* outputBuffer;
    int stride = 0;
    uiEmu->addVideo((uint8_t**)&outputBuffer, &stride, {_width, _height} ,{_width, _height});
    memset(outputBuffer, 0xFF, _width * _height * sizeof(color_t));
    retro_set_video_buffer(outputBuffer, _width);

    std::string romPath = game.romsPath + game.path;

    struct retro_game_info game_info = {0};
    game_info.path = romPath.c_str();
    if (!retro_load_game(&game_info)) {
        return -1;
    }

    memset(_savedata, 0xFF, sizeof(_savedata));

    // try to load savedata from .sav
    _savePath = uiEmu->getUi()->getIo()->getDataPath() + "save/" + Utility::removeExt(game.path) + ".sav";
    if (std::filesystem::exists(_savePath)) {
        std::ifstream file;
        file.open(_savePath, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streampos fileSize = file.tellg();        
            file.seekg(0, std::ios::beg);

            if (fileSize > SIZE_CART_FLASH1M) {
                printf("Warning : %s is too big (%lld), reset to %d\n", _savePath.c_str(), fileSize, SIZE_CART_FLASH1M);
                fileSize = SIZE_CART_FLASH1M;
            }

            if (!file.read(_savedata, fileSize)) {
                printf("Warning : failed to load %s\n", _savePath.c_str());
                memset(_savedata, 0xFF, sizeof(_savedata));
            }
            file.close();
        }
    }

    struct retro_system_av_info mgba_av_info = {0};
    retro_get_system_av_info(&mgba_av_info);
    uiEmu->addAudio((int)mgba_av_info.timing.sample_rate, (int)(mgba_av_info.timing.sample_rate/mgba_av_info.timing.fps));

    _running = true;
    return 0;
}

void mgba_stop() {
    if (!_running) {
        return;
    }

    _running = false;
    size_t savSize = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
    if (savSize > SIZE_CART_FLASH1M) {
        printf("Warning : savedata size (%zu) is bigger than %d\n", savSize, SIZE_CART_FLASH1M);
        return;
    }

    retro_unload_game();
    _width  = MGBA_WIDTH_PIXEL;
    _height = MGBA_HEIGHT_PIXEL;

    // write savedata to .sav
    std::ofstream file(_savePath.c_str(), std::ios::binary | std::ios::trunc);
    if (file) {
        file.write(_savedata, savSize);
        file.close();
    }
}

void mgbp_step() {
    retro_run();
    uiEmu->getVideo()->unlock();
}
