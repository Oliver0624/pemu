//
// Created by Oliver on 04/25/2024
//

#include "c2dui.h"
#include "uiEmu.h"

extern "C" {
#include "mgba-util/vfs.h"
#include "mgba/core/blip_buf.h"
#include "mgba/core/config.h"
#include "mgba/core/core.h"
#include "mgba/core/directories.h"
#include "mgba/core/serialize.h"
#include "mgba/internal/gba/input.h"
#include "mgba/internal/gb/gb.h"
}

#define SAMPLE_RATE 44100
#define PGBA_INPUT_BINDING 0x47434E31
#define SAMPLES_PER_FRAME_MOVING_AVG_ALPHA (1.0f / 180.0f)

using namespace c2d;
using namespace c2dui;

mCore *s_core = nullptr;

namespace {
    VDir *s_vDir = nullptr;
    VFile *s_vFile = nullptr;
    int16_t *audioSampleBuffer = nullptr;
    float audioSamplesPerFrameAvg = 0.0f;
    size_t audioSampleBufferSize = 0;

    void cleanupCoreLoadFailure() {
        if (!s_core) {
            return;
        }

        mInputMapDeinit(&s_core->inputMap);
        mCoreConfigDeinit(&s_core->config);
        s_core->deinit(s_core);
        s_core = nullptr;
    }
}

PGBAUiEmu::PGBAUiEmu(UiMain *ui) : UiEmu(ui) {
    printf("PGBAGuiEmu()\n");
}

int PGBAUiEmu::load(const ss_api::Game &game) {
    currentGame = game;
    getUi()->getUiProgressBox()->setTitle(game.name);
    getUi()->getUiProgressBox()->setMessage(TEXT_MSG_PLEASE_WAIT);
    getUi()->getUiProgressBox()->setProgress(0);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Visible);
    getUi()->getUiProgressBox()->setLayer(1000);
    getUi()->flip();

    std::string path = game.romsPath + game.path;
    s_core = mCoreFind(path.c_str());
    if (!s_core) {
        getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);
        getUi()->getUiMessageBox()->show(TEXT_MSG_TITTLE_ERROR, TEXT_MSG_INVALID_ROM, TEXT_BUTTON_OK);
        return -1;
    }

    mCoreInitConfig(s_core, nullptr);
    s_core->init(s_core);

    mInputMapInit(&s_core->inputMap, &GBAInputInfo);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::A), GBA_KEY_A);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::B), GBA_KEY_B);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::LB), GBA_KEY_L);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::RB), GBA_KEY_R);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::Start), GBA_KEY_START);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::Select), GBA_KEY_SELECT);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::Up), GBA_KEY_UP);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::Down), GBA_KEY_DOWN);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::Left), GBA_KEY_LEFT);
    mInputBindKey(&s_core->inputMap, PGBA_INPUT_BINDING, __builtin_ctz(Input::Button::Right), GBA_KEY_RIGHT);
    mInputMapLoad(&s_core->inputMap, PGBA_INPUT_BINDING, mCoreConfigGetInput(&s_core->config));

    mCoreOptions coreOptions = {
            .useBios = true,
            .logLevel = 0x01 | 0x02 | 0x04,
            .rewindEnable = false,
            .rewindBufferCapacity = 600,
            .audioBuffers = 1024,
            .volume = 0x100,
            .videoSync = false,
            .audioSync = true
    };
    mCoreConfigLoadDefaults(&s_core->config, &coreOptions);
    mCoreLoadConfig(s_core);

    auto dataPath = getUi()->getIo()->getDataPath();
    std::string savePath = dataPath + "save";
    std::string patchPath = dataPath + "patches";
    std::string cheatPath = dataPath + "cheats";
    mCoreOptions dirOptions = {
            .savegamePath = const_cast<char *>(savePath.c_str()),
            .patchPath = const_cast<char *>(patchPath.c_str()),
            .cheatsPath = const_cast<char *>(cheatPath.c_str())
    };
    mDirectorySetMapOptions(&s_core->dirs, &dirOptions);

    if (!mCoreLoadFile(s_core, path.c_str())) {
        cleanupCoreLoadFailure();
        getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);
        getUi()->getUiMessageBox()->show(TEXT_MSG_TITTLE_ERROR, TEXT_MSG_INVALID_ROM, TEXT_BUTTON_OK);
        return -1;
    }

    unsigned width = 0;
    unsigned height = 0;
    if (s_core->platform(s_core) == mPLATFORM_GB) {
        auto *gb = (struct GB *) s_core->board;
        uint8_t sgbFlag = gb->memory.rom[0x146];
        uint8_t oldLicensee = gb->memory.rom[0x14B];
        bool isSGBRom = (sgbFlag == 0x03 && oldLicensee == 0x33);
        width = isSGBRom ? 256 : 160;
        height = isSGBRom ? 224 : 144;
    } else {
        s_core->desiredVideoDimensions(s_core, &width, &height);
    }

    uint8_t *buffer = nullptr;
    int pitch = 0;
    addVideo(&buffer, &pitch, {(int) width, (int) height}, {(int) width, (int) height});
    s_core->setVideoBuffer(s_core, reinterpret_cast<color_t *>(buffer), width);

    auto audioSamplesPerFrame = (size_t) ((float) SAMPLE_RATE * (float) s_core->frameCycles(s_core) /
                                          (float) s_core->frequency(s_core));
    audioSampleBufferSize = audioSamplesPerFrame * 2;
    audioSampleBuffer = static_cast<int16_t *>(malloc(audioSampleBufferSize * sizeof(int16_t)));
    audioSamplesPerFrameAvg = (float) audioSamplesPerFrame;
    size_t internalAudioBufferSize = audioSamplesPerFrame * 2;
    if (internalAudioBufferSize > 0x4000) {
        internalAudioBufferSize = 0x4000;
    }
    s_core->setAudioBufferSize(s_core, internalAudioBufferSize);
    blip_set_rates(s_core->getAudioChannel(s_core, 0), s_core->frequency(s_core), SAMPLE_RATE);
    blip_set_rates(s_core->getAudioChannel(s_core, 1), s_core->frequency(s_core), SAMPLE_RATE);
    addAudio(SAMPLE_RATE, Audio::toSamples(SAMPLE_RATE, 60));

    path = getUi()->getIo()->getDataPath() + "bios/gba_bios.bin";
    if (getUi()->getIo()->exist(path)) {
        VFile *bios = VFileOpen(path.c_str(), O_RDONLY);
        if (bios) {
            s_core->loadBIOS(s_core, bios, 0);
        }
    } else {
        mCoreConfigSetIntValue(&s_core->config, "useBios", 0);
        s_core->reloadConfigOption(s_core, "useBios", nullptr);
    }

    getUi()->getUiProgressBox()->setProgress(1);
    getUi()->flip();
    getUi()->delay(500);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);

    s_core->reset(s_core);
    mCoreAutoloadSave(s_core);
    mCoreAutoloadCheats(s_core);
    mCoreAutoloadPatch(s_core);

    return UiEmu::load(game);
}

void PGBAUiEmu::stop() {
    saveAutoState();

    if (s_core) {
        mInputMapDeinit(&s_core->inputMap);
        mCoreConfigDeinit(&s_core->config);
        s_core->unloadROM(s_core);
        s_core->deinit(s_core);
        s_core = nullptr;
    }

    if (s_vDir) {
        s_vDir->close(s_vDir);
        s_vDir = nullptr;
    } else if (s_vFile) {
        s_vFile->close(s_vFile);
        s_vFile = nullptr;
    }

    if (audioSampleBuffer) {
        free(audioSampleBuffer);
        audioSampleBuffer = nullptr;
        audioSampleBufferSize = 0;
        audioSamplesPerFrameAvg = 0.0f;
    }

    UiEmu::stop();
}

bool PGBAUiEmu::onInput(c2d::Input::Player *players) {
    return UiEmu::onInput(players);
}

void PGBAUiEmu::onUpdate() {
    if (!isPaused() && s_core) {
        const auto buttons = getUi()->getInput()->getButtons(0);
        const auto keys = mInputMapKeyBits(&s_core->inputMap, PGBA_INPUT_BINDING, buttons, 0);
        s_core->setKeys(s_core, keys);

        s_core->runFrame(s_core);
        video->unlock();

        if (!audioSampleBuffer) {
            return UiEmu::onUpdate();
        }

        blip_t *audioChannelLeft = s_core->getAudioChannel(s_core, 0);
        blip_t *audioChannelRight = s_core->getAudioChannel(s_core, 1);
        int samplesAvail = blip_samples_avail(audioChannelLeft);
        if (samplesAvail > 0) {
            audioSamplesPerFrameAvg = (SAMPLES_PER_FRAME_MOVING_AVG_ALPHA * (float) samplesAvail) +
                                      ((1.0f - SAMPLES_PER_FRAME_MOVING_AVG_ALPHA) * audioSamplesPerFrameAvg);
            size_t samplesToRead = (size_t) audioSamplesPerFrameAvg;
            if (audioSampleBufferSize < samplesToRead * 2) {
                audioSampleBufferSize = samplesToRead * 2;
                audioSampleBuffer = (int16_t *) realloc(audioSampleBuffer, audioSampleBufferSize * sizeof(int16_t));
                if (!audioSampleBuffer) {
                    return UiEmu::onUpdate();
                }
            }
            int produced = blip_read_samples(audioChannelLeft, audioSampleBuffer, (int) samplesToRead, true);
            blip_read_samples(audioChannelRight, audioSampleBuffer + 1, (int) samplesToRead, true);
            if (produced > 0) {
                audio->play(audioSampleBuffer, produced);
            }
        }
    }

    return UiEmu::onUpdate();
}
