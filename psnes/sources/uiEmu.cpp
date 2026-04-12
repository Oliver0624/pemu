//
// Created by cpasjuste on 01/06/18.
//

#include "c2dui.h"
#include "uiEmu.h"

#include <snes9x.h>
#include <memmap.h>
#include <apu/apu.h>
#include <controls.h>
#include <conffile.h>
#include <display.h>
#include <sys/stat.h>
#include <cheats.h>

#ifdef __PSP2__
#include <psp2/io/dirent.h>
#include <psp2/kernel/threadmgr.h>
#define usleep sceKernelDelayThread
#endif

using namespace c2d;
using namespace c2dui;

static UiMain *m_ui;

static Option *optionAudioSync = nullptr;

static void *audio_buffer = nullptr;

static const char *s9x_base_dir = nullptr;

static char default_dir[PATH_MAX + 1];

static constexpr uint32 kSnesPointer1 = 0x100;
static constexpr uint32 kSnesPointer2 = 0x101;
static constexpr uint32 kSnesMouseLeft1 = 0x110;
static constexpr uint32 kSnesMouseRight1 = 0x111;
static constexpr uint32 kSnesMouseLeft2 = 0x112;
static constexpr uint32 kSnesMouseRight2 = 0x113;
static constexpr uint32 kSnesScopeFire = 0x120;
static constexpr uint32 kSnesScopeCursor = 0x121;
static constexpr uint32 kSnesScopePause = 0x122;
static constexpr uint32 kSnesScopeTurbo = 0x123;
static constexpr uint32 kSnesJustifierTrigger = 0x130;
static constexpr uint32 kSnesJustifierStart = 0x131;
static constexpr uint32 kSnesJustifierOffscreenTrigger = 0x132;

static const char dirNames[13][32] = {
        "",             // DEFAULT_DIR
        "",             // HOME_DIR
        "",             // ROMFILENAME_DIR
        "roms",          // ROM_DIR
        "sram",         // SRAM_DIR
        "saves",        // SNAPSHOT_DIR
        "screenshots",  // SCREENSHOT_DIR
        "spc",          // SPC_DIR
        "cheat",        // CHEAT_DIR
        "patch",        // PATCH_DIR
        "bios",         // BIOS_DIR
        "log",          // LOG_DIR
        ""
};

static int S9xCreateDirectory();

std::string getButtonId(int player, const std::string &name) {
    return "Joypad" + std::to_string(player) + " " + name;
}

static void mapSnesInput() {
    S9xUnmapAllControls();

    for (int i = 0; i < 4; i++) {
        S9xMapButton(0 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Up").c_str()), false);
        S9xMapButton(1 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Down").c_str()), false);
        S9xMapButton(2 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Left").c_str()), false);
        S9xMapButton(3 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Right").c_str()), false);
        S9xMapButton(4 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "A").c_str()), false);
        S9xMapButton(5 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "B").c_str()), false);
        S9xMapButton(6 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "X").c_str()), false);
        S9xMapButton(7 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Y").c_str()), false);
        S9xMapButton(8 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "L").c_str()), false);
        S9xMapButton(9 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "R").c_str()), false);
        S9xMapButton(10 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Start").c_str()), false);
        S9xMapButton(11 + (i * 12), S9xGetCommandT(getButtonId(i + 1, "Select").c_str()), false);
    }

    S9xMapPointer(kSnesPointer1, S9xGetCommandT("Pointer Mouse1+Superscope+Justifier1+MacsRifle"),
                  false);
    S9xMapPointer(kSnesPointer2, S9xGetCommandT("Pointer Mouse2+Justifier2"), false);
    S9xMapButton(kSnesMouseLeft1, S9xGetCommandT("Mouse1 L"), false);
    S9xMapButton(kSnesMouseRight1, S9xGetCommandT("Mouse1 R"), false);
    S9xMapButton(kSnesMouseLeft2, S9xGetCommandT("Mouse2 L"), false);
    S9xMapButton(kSnesMouseRight2, S9xGetCommandT("Mouse2 R"), false);
    S9xMapButton(kSnesScopeFire, S9xGetCommandT("Superscope Fire"), false);
    S9xMapButton(kSnesScopeCursor, S9xGetCommandT("Superscope Cursor"), false);
    S9xMapButton(kSnesScopePause, S9xGetCommandT("Superscope Pause"), false);
    S9xMapButton(kSnesScopeTurbo, S9xGetCommandT("Superscope ToggleTurbo"), false);
    S9xMapButton(kSnesJustifierTrigger, S9xGetCommandT("Justifier1 Trigger"), false);
    S9xMapButton(kSnesJustifierStart, S9xGetCommandT("Justifier1 Start"), false);
    S9xMapButton(kSnesJustifierOffscreenTrigger,
                 S9xGetCommandT("Justifier1 AimOffscreen Trigger"), false);
}

static void setupSnesControllers() {
    S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
    S9xSetController(1, CTL_JOYPAD, 1, 0, 0, 0);

    Option *controllerOption = m_ui ? m_ui->getConfig()->get(Option::ROM_PSNES_CONTROLLER, true) : nullptr;
    std::string controllerMode = controllerOption ? controllerOption->getValueString() : "AUTO";
    if (controllerMode == "JOYPAD") {
        S9xReportControllers();
        return;
    }
    if (controllerMode == "MOUSE") {
        S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
        S9xReportControllers();
        return;
    }
    if (controllerMode == "SUPERSCOPE") {
        S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
        S9xReportControllers();
        return;
    }
    if (controllerMode == "JUSTIFIER") {
        S9xSetController(1, CTL_JUSTIFIER, 1, 0, 0, 0);
        S9xReportControllers();
        return;
    }

    if (strncmp((const char *) Memory.NSRTHeader + 24, "NSRT", 4) != 0) {
        S9xReportControllers();
        return;
    }

    switch (Memory.NSRTHeader[29]) {
        case 0x10:
        case 0x20:
            S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
            break;

        case 0x01:
        case 0x08:
            S9xSetController(1, CTL_MOUSE, 1, 0, 0, 0);
            break;

        case 0x03:
        case 0x04:
            S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
            break;

        case 0x05:
            S9xSetController(1, CTL_JUSTIFIER, 1, 0, 0, 0);
            break;

        case 0x06:
            S9xSetController(1, CTL_MP5, 1, 2, 3, 4);
            break;

        case 0x22:
            S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
            S9xSetController(1, CTL_MOUSE, 1, 0, 0, 0);
            break;

        case 0x24:
        case 0x27:
            S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
            S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
            break;

        case 0x66:
            S9xSetController(0, CTL_MP5, 0, 1, 2, 3);
            S9xSetController(1, CTL_MP5, 4, 5, 6, 7);
            break;

        default:
            break;
    }

    S9xReportControllers();
}

static bool mapPointerToSnesVideo(C2DUIVideo *video, const c2d::Input::Pointer &pointer, int16 &x, int16 &y) {
    if (video == nullptr || !pointer.active) {
        return false;
    }

    auto bounds = video->getGlobalBounds();
    if (bounds.width <= 0 || bounds.height <= 0) {
        return false;
    }

    float localX = pointer.position.x - bounds.left;
    float localY = pointer.position.y - bounds.top;
    if (localX < 0 || localX > bounds.width || localY < 0 || localY > bounds.height) {
        return false;
    }

    float mappedX = (localX * (float) video->getTextureRect().width) / bounds.width;
    float mappedY = (localY * (float) video->getTextureRect().height) / bounds.height;

    if (mappedX < 0) {
        mappedX = 0;
    } else if (mappedX >= video->getTextureRect().width) {
        mappedX = (float) video->getTextureRect().width - 1;
    }
    if (mappedY < 0) {
        mappedY = 0;
    } else if (mappedY >= video->getTextureRect().height) {
        mappedY = (float) video->getTextureRect().height - 1;
    }

    x = (int16) mappedX;
    y = (int16) mappedY;
    return true;
}

static bool portHasController(int port, controllers controller, int8 *id = nullptr) {
    controllers current = CTL_NONE;
    int8 currentId1 = -1;
    int8 currentId2 = -1;
    int8 currentId3 = -1;
    int8 currentId4 = -1;
    S9xGetController(port, &current, &currentId1, &currentId2, &currentId3, &currentId4);
    if (id != nullptr) {
        *id = currentId1;
    }
    return current == controller;
}

PSNESUiEmu::PSNESUiEmu(UiMain *ui) : UiEmu(ui) {
    printf("PSNESUIEmu()\n");
    m_ui = ui;
}

int PSNESUiEmu::load(const ss_api::Game &game) {
    pendingMenuAction = MenuAction::None;
    superScopeTurboEnabled = false;

    pMain->getUiProgressBox()->setTitle(game.name);
    pMain->getUiProgressBox()->setMessage(TEXT_MSG_PLEASE_WAIT);
    pMain->getUiProgressBox()->setProgress(0);
    pMain->getUiProgressBox()->setVisibility(Visibility::Visible);
    pMain->getUiProgressBox()->setLayer(1000);
    pMain->flip();

    strncpy(default_dir, pMain->getIo()->getDataPath().c_str(), PATH_MAX);
    s9x_base_dir = default_dir;

    memset(&Settings, 0, sizeof(Settings));
    S9xLoadConfigFiles(nullptr, 0);

    // override S9xLoadConfigFiles
    Settings.MouseMaster = TRUE;
    Settings.SuperScopeMaster = TRUE;
    Settings.JustifierMaster = TRUE;
    Settings.MultiPlayer5Master = TRUE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    // audio
    Settings.SixteenBitSound = TRUE;
    Settings.Stereo = TRUE;
    Settings.SoundSync = TRUE;
    Settings.SoundInputRate = 31950;
    Settings.SoundPlaybackRate = 48000;

    // audio
    Settings.Transparency =
            pMain->getConfig()->get(Option::ROM_PSNES_TRANSPARENCY, true)->getIndex();
    Settings.AutoDisplayMessages =
            pMain->getConfig()->get(Option::ROM_PSNES_DISPLAY_MESSAGES, true)->getIndex();
    Settings.InitialInfoStringTimeout = 120;
    Settings.HDMATimingHack = 100;
    Settings.BlockInvalidVRAMAccessMaster =
            pMain->getConfig()->get(Option::ROM_PSNES_BLOCK_VRAM, true)->getIndex();
    Settings.StopEmulation = TRUE;
    Settings.WrongMovieStateProtection = TRUE;
    Settings.DumpStreamsMaxFrames = -1;
    Settings.StretchScreenshots = 1;
    Settings.SnapshotScreenshots = FALSE;
    Settings.DontSaveOopsSnapshot = TRUE;
    Settings.FastSavestates = TRUE;
    Settings.SeparateEchoBuffer = FALSE;

    int skipFramesCfg = pMain->getConfig()->get(Option::ROM_PSNES_FRAMESKIP, true)->getIndex();
    if (skipFramesCfg == 0) {
        Settings.SkipFrames = 0;
    } else if (skipFramesCfg == 1) {
        Settings.SkipFrames = AUTO_FRAMERATE;
    } else {
        Settings.SkipFrames = skipFramesCfg - 1;
    }
    printf("Settings.SkipFrames: %i\n", Settings.SkipFrames);
    Settings.TurboMode = pMain->getConfig()->get(Option::ROM_PSNES_TURBO_MODE, true)->getIndex();
    Settings.TurboSkipFrames = pMain->getConfig()->get(Option::ROM_PSNES_TURBO_FRAMESKIP,
                                                       true)->getIndex();
    Settings.CartAName[0] = 0;
    Settings.CartBName[0] = 0;

    CPU.Flags = 0;

    S9xCreateDirectory();

    if (!Memory.Init() || !S9xInitAPU()) {
        Memory.Deinit();
        S9xDeinitAPU();
        printf("Could not initialize Snes9x Memory.\n");
        pMain->getUiProgressBox()->setVisibility(Visibility::Hidden);
        stop();
        return -1;
    }

    S9xInitSound(32);
    S9xSetSoundMute(FALSE);
    S9xSetSamplesAvailableCallback(nullptr, nullptr);

    mapSnesInput();

    uint32 saved_flags = CPU.Flags;

    std::string fullPath = game.romsPath + game.path;
    printf("Memory.LoadROM: %s\n", fullPath.c_str());
    if (!Memory.LoadROM(fullPath.c_str())) {
        printf("Could not open ROM: %s\n", fullPath.c_str());
        pMain->getUiProgressBox()->setVisibility(Visibility::Hidden);
        pMain->getUiMessageBox()->show(TEXT_MSG_TITTLE_ERROR, TEXT_MSG_INVALID_ROM, TEXT_BUTTON_OK);
        stop();
        return -1;
    }

    Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());

    Settings.ApplyCheats = pMain->getConfig()->get(Option::ROM_PSNES_CHEATS, true)->getIndex() == 1;
    S9xDeleteCheats();
    S9xCheatsEnable();
    if (Settings.ApplyCheats) {
        printf("S9xLoadCheatFile(%s)\n", S9xGetFilename(".cht", CHEAT_DIR).c_str());
        S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
    }

    CPU.Flags = saved_flags;
    Settings.StopEmulation = FALSE;
    setupSnesControllers();

    // audio
    int samples = Audio::toSamples((int) Settings.SoundPlaybackRate,
                                   (float) Memory.ROMFramesPerSecond);
    addAudio((int) Settings.SoundPlaybackRate, samples * 2);
    audio_buffer = malloc(getAudio()->getSamplesSize() * getAudio()->getChannels() * 5);
    optionAudioSync = pMain->getConfig()->get(Option::Id::ROM_AUDIO_SYNC, true);

    // video
    S9xGraphicsInit();

    addVideo((uint8_t **) &GFX.Screen, (int *) &GFX.Pitch, {MAX_SNES_WIDTH, MAX_SNES_HEIGHT});
    targetFps = (float) Memory.ROMFramesPerSecond;

    pMain->getUiProgressBox()->setProgress(1);
    pMain->flip();
    pMain->delay(500);
    pMain->getUiProgressBox()->setVisibility(Visibility::Hidden);

    return UiEmu::load(game);
}

void PSNESUiEmu::stop() {
    Settings.StopEmulation = TRUE;
    pendingMenuAction = MenuAction::None;
    superScopeTurboEnabled = false;
    saveAutoState();

    Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());

    S9xUnmapAllControls();
    S9xGraphicsDeinit();
    Memory.Deinit();
    S9xDeinitAPU();

    if (audio_buffer) {
        free(audio_buffer);
    }

    UiEmu::stop();
}

bool PSNESUiEmu::onInput(c2d::Input::Player *players) {
    return UiEmu::onInput(players);
}

void PSNESUiEmu::onUpdate() {
    UiEmu::onUpdate();

    if (isVisible() && !isPaused()) {
        auto players = pMain->getInput()->getPlayers();

        // update snes9x buttons
        for (uint32 i = 0; i < 4; i++) {
            unsigned int buttons = players[i].buttons;
            S9xReportButton(0 + (i * 12), (buttons & Input::Button::Up) > 0);
            S9xReportButton(1 + (i * 12), (buttons & Input::Button::Down) > 0);
            S9xReportButton(2 + (i * 12), (buttons & Input::Button::Left) > 0);
            S9xReportButton(3 + (i * 12), (buttons & Input::Button::Right) > 0);
            S9xReportButton(4 + (i * 12), (buttons & Input::Button::A) > 0);
            S9xReportButton(5 + (i * 12), (buttons & Input::Button::B) > 0);
            S9xReportButton(6 + (i * 12), (buttons & Input::Button::X) > 0);
            S9xReportButton(7 + (i * 12), (buttons & Input::Button::Y) > 0);
            S9xReportButton(8 + (i * 12), (buttons & Input::Button::LT) > 0);
            S9xReportButton(9 + (i * 12), (buttons & Input::Button::RT) > 0);
            S9xReportButton(10 + (i * 12), (buttons & Input::Button::Start) > 0);
            S9xReportButton(11 + (i * 12), (buttons & Input::Button::Select) > 0);
        }

        int16 pointerX = 0;
        int16 pointerY = 0;
        bool pointerOnScreen = mapPointerToSnesVideo(video, players[0].pointer, pointerX, pointerY);
        bool primaryPressed = players[0].pointer.active &&
                              (players[0].pointer.buttons & Input::Pointer::Primary) != 0;
        bool secondaryPressed = (players[0].buttons & Input::Button::A) != 0 ||
                                (players[0].pointer.buttons & Input::Pointer::Secondary) != 0;
        bool menuScopeTurbo = pendingMenuAction == MenuAction::SuperScopeTurbo;
        pendingMenuAction = MenuAction::None;

        S9xReportButton(kSnesMouseLeft1, false);
        S9xReportButton(kSnesMouseRight1, false);
        S9xReportButton(kSnesMouseLeft2, false);
        S9xReportButton(kSnesMouseRight2, false);
        S9xReportButton(kSnesScopeFire, false);
        S9xReportButton(kSnesScopeCursor, false);
        S9xReportButton(kSnesScopePause, false);
        S9xReportButton(kSnesScopeTurbo, false);
        S9xReportButton(kSnesJustifierTrigger, false);
        S9xReportButton(kSnesJustifierStart, false);
        S9xReportButton(kSnesJustifierOffscreenTrigger, false);

        int8 mouseId = -1;
        if (portHasController(0, CTL_MOUSE, &mouseId) || portHasController(1, CTL_MOUSE, &mouseId)) {
            if (pointerOnScreen) {
                S9xReportPointer(kSnesPointer1, pointerX, pointerY);
                S9xReportPointer(kSnesPointer2, pointerX, pointerY);
            }
            const uint32 leftId = mouseId == 1 ? kSnesMouseLeft2 : kSnesMouseLeft1;
            const uint32 rightId = mouseId == 1 ? kSnesMouseRight2 : kSnesMouseRight1;
            S9xReportButton(leftId, (players[0].buttons & Input::Button::LT) != 0);
            S9xReportButton(rightId, (players[0].buttons & Input::Button::LB) != 0);
        }

        if (isSuperScopeConnected()) {
            if (pointerOnScreen) {
                S9xReportPointer(kSnesPointer1, pointerX, pointerY);
                S9xReportPointer(kSnesPointer2, pointerX, pointerY);
            }
            S9xReportButton(kSnesScopeFire, pointerOnScreen && primaryPressed);
            S9xReportButton(kSnesScopeCursor, secondaryPressed);
            S9xReportButton(kSnesScopePause, (players[0].buttons & Input::Button::Start) != 0);
            S9xReportButton(kSnesScopeTurbo, menuScopeTurbo);
        }

        if (isJustifierConnected()) {
            if (pointerOnScreen) {
                S9xReportPointer(kSnesPointer1, pointerX, pointerY);
                S9xReportPointer(kSnesPointer2, pointerX, pointerY);
            }
            S9xReportButton(kSnesJustifierTrigger, pointerOnScreen && primaryPressed);
            S9xReportButton(kSnesJustifierStart, secondaryPressed);
            S9xReportButton(kSnesJustifierOffscreenTrigger,
                            players[0].pointer.active && !pointerOnScreen && primaryPressed);
        }

        S9xMainLoop();
    }
}

void PSNESUiEmu::pause() {
    S9xSetSoundMute(TRUE);
    S9xClearSamples();
    UiEmu::pause();
}

void PSNESUiEmu::resume() {
    S9xSetSoundMute(FALSE);
    UiEmu::resume();
}

void PSNESUiEmu::triggerMenuAction(MenuAction action) {
    pendingMenuAction = action;
}

bool PSNESUiEmu::isSuperScopeTurboEnabled() const {
    return superScopeTurboEnabled;
}

void PSNESUiEmu::setSuperScopeTurboEnabled(bool enabled) {
    if (superScopeTurboEnabled == enabled) {
        return;
    }

    superScopeTurboEnabled = enabled;
    pendingMenuAction = MenuAction::SuperScopeTurbo;
}

static bool isControllerConnected(controllers controller) {
    return portHasController(0, controller) || portHasController(1, controller);
}

bool PSNESUiEmu::isSuperScopeConnected() const {
    return ::isControllerConnected(CTL_SUPERSCOPE);
}

bool PSNESUiEmu::isMouseConnected() const {
    return ::isControllerConnected(CTL_MOUSE);
}

bool PSNESUiEmu::isJustifierConnected() const {
    return ::isControllerConnected(CTL_JUSTIFIER);
}

///////////////////////////////////////////////////////////////////////////////
// Functions called by snes9x below
///////////////////////////////////////////////////////////////////////////////

void S9xSyncSpeed() {
    if (Settings.Mute) {
        S9xClearSamples();
        return;
    }

    auto audio = m_ui->getUiEmu()->getAudio();
    int samples = S9xGetSampleCount();
    S9xMixSamples((uint8 *) audio_buffer, samples);
#if 0
    int queued = audio->getSampleBufferQueued();
    int capacity = audio->getSampleBufferCapacity();
    if (samples + queued > capacity) {
        printf("WARNING: samples: %i, queued: %i, capacity: %i (fps: %i)\n",
               samples, queued, capacity, Memory.ROMFramesPerSecond);
    }
#endif
    Audio::SyncMode mode = Memory.ROMFramesPerSecond < 60 || optionAudioSync->getValueBool() ?
                           Audio::SyncMode::Safe : Audio::SyncMode::None;
    audio->play(audio_buffer, samples >> 1, mode);
}

bool8 S9xInitUpdate() {
    return TRUE;
}

bool8 S9xDeinitUpdate(int width, int height) {
    //printf("S9xDeinitUpdate(%i, %i\n", width, height);
    C2DUIVideo *video = m_ui->getUiEmu()->getVideo();
    if (video) {
        auto rect = video->getTextureRect();
        if (rect.width != width || rect.height != height) {
            video->setSize((float) width, (float) height);
            video->setTextureRect({0, 0, width, height});
            video->updateScaling();
        }
        video->unlock();
    }

    return TRUE;
}

bool8 S9xContinueUpdate(int width, int height) {
    S9xDeinitUpdate(width, height);
    return TRUE;
}

static int S9xCreateDirectory() {
    if (strlen(s9x_base_dir) + 1 + sizeof(dirNames[0]) > PATH_MAX + 1)
        return (-1);

    m_ui->getIo()->create(s9x_base_dir);

    for (int i = 0; i < LAST_DIR; i++) {
        if (dirNames[i][0]) {
            char s[PATH_MAX + 1];
            snprintf(s, PATH_MAX + 1, "%s%s%s", s9x_base_dir, SLASH_STR, dirNames[i]);
            m_ui->getIo()->create(s);
        }
    }

    return (0);
}

std::string S9xGetDirectory(s9x_getdirtype type) {
    switch (type) {
        case ROMFILENAME_DIR:
            return Memory.ROMFilename;
        default:
            return s9x_base_dir;
    }
}

bool8 S9xOpenSnapshotFile(const char *filename, bool8 read_only, STREAM*file) {
    if ((*file = OPEN_STREAM(filename, read_only ? "rb" : "wb"))) return (TRUE);
    return (FALSE);
}

void S9xCloseSnapshotFile(STREAM file) {
    CLOSE_STREAM(file);
}

void S9xAutoSaveSRAM() {
    Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());
}

void S9xExit() {
    m_ui->getUiEmu()->stop();
}

void S9xMessage(int type, int number, const char *message) {
    const int max = 36 * 3;
    static char buffer[max + 1];
    printf("[%i][%i]: %s\n", type, number, message);
    strncpy(buffer, message, max + 1);
    buffer[max] = 0;
    S9xSetInfoString(buffer);
}

std::string S9xGetFilenameInc(std::string in, s9x_getdirtype) { return ""; }

const char *S9xStringInput(const char *message) { return nullptr; }

void S9xSetPalette() {}

bool8 S9xOpenSoundDevice() { return TRUE; }

void S9xToggleSoundChannel(int c) {}

void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2) {}

void S9xExtraUsage() {}

void S9xParseArg(char **argv, int &i, int argc) {}

void S9xParsePortConfig(ConfigFile &conf, int pass) {}

bool S9xPollButton(uint32 id, bool *pressed) { return false; }

bool S9xPollAxis(uint32 id, int16 *value) { return false; }

bool S9xPollPointer(uint32 id, int16 *x, int16 *y) { return false; }

const char *S9xChooseFilename(bool8 read_only) { return nullptr; }

const char *S9xChooseMovieFilename(bool8 read_only) { return nullptr; }
