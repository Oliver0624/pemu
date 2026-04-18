//
// Created by cpasjuste on 01/06/18.
//

#include <algorithm>
#include <fstream>
#include <sstream>

#include "fltkui/nstcommon.h"
#include "fltkui/video.h"
#include "fltkui/config.h"
#include "fltkui/audio.h"
#include "fltkui/input.h"

#include "c2dui.h"
#include "uiEmu.h"

extern PNESUiEmu *uiEmu;

/// NESTOPIA
settings_t conf;

extern nstpaths_t nstpaths;

extern bool (*nst_archive_select)(const char *, char *, size_t);

extern Nes::Core::Input::Controllers *cNstPads;

extern Emulator emulator;
/// NESTOPIA

int nestopia_state_load(const char *path);
int nestopia_state_save(const char *path);

namespace {
    bool nestopia_state_load_memory(const std::vector<uint8_t> &data) {
        if (data.empty()) return false;
        std::string stateData(data.begin(), data.end());
        std::istringstream is(stateData, std::ios::binary);
        Machine machine(emulator);
        return machine.LoadState(is) == 0;
    }

    bool nestopia_state_save_memory(std::vector<uint8_t> &data) {
        std::ostringstream os(std::ios::binary);
        Machine machine(emulator);
        if (machine.SaveState(os, Nes::Api::Machine::NO_COMPRESSION) != 0) return false;
        
        // Use the underlying string directly to avoid one copy if possible, 
        // though ostringstream::str() still creates a copy. 
        // For a more complete fix we'd need a custom ostream.
        std::string s = os.str();
        data.assign(s.begin(), s.end());
        return !data.empty();
    }
}

static bool mapPointerToNesVideo(c2dui::C2DUIVideo *video, const c2d::Input::Pointer &pointer,
                                 uint &x, uint &y) {
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

    float mappedX = (localX * (float) Video::Output::WIDTH) / bounds.width;
    float mappedY = (localY * (float) Video::Output::HEIGHT) / bounds.height;

    if (mappedX < 0) {
        mappedX = 0;
    } else if (mappedX >= Video::Output::WIDTH) {
        mappedX = (float) Video::Output::WIDTH - 1;
    }
    if (mappedY < 0) {
        mappedY = 0;
    } else if (mappedY >= Video::Output::HEIGHT) {
        mappedY = (float) Video::Output::HEIGHT - 1;
    }

    x = (uint) mappedX;
    y = (uint) mappedY;
    return true;
}

PNESUiEmu::PNESUiEmu(UiMain *ui) : UiEmu(ui) {
    printf("PNESGuiEmu()\n");
}

int PNESUiEmu::load(const ss_api::Game &game) {
    getUi()->getUiProgressBox()->setTitle(game.name);
    getUi()->getUiProgressBox()->setMessage(TEXT_MSG_PLEASE_WAIT);
    getUi()->getUiProgressBox()->setProgress(0);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Visible);
    getUi()->getUiProgressBox()->setLayer(1000);
    getUi()->flip();

    // default config
    nestopia_config_init();

    std::string fullPath = game.romsPath + game.path;
    if (nestopia_core_init(fullPath.c_str()) != 0) {
        getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);
        getUi()->getUiMessageBox()->show(TEXT_MSG_TITTLE_ERROR, TEXT_MSG_PNES_INVALID_FILE, TEXT_BUTTON_OK);
        stop();
        return -1;
    }

    targetFps = nst_pal() ? 50 : 60;

    getUi()->getUiProgressBox()->setProgress(1);
    getUi()->flip();
    getUi()->delay(500);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);

    initRewindUi();

    return UiEmu::load(game);
}

void PNESUiEmu::stop() {
    resetRewind();
    nst_pause();
    saveAutoState();

    // Remove the cartridge and shut down the NES
    nst_unload();

    // Unload the FDS BIOS, NstDatabase.xml, and the custom palette
    nst_db_unload();
    nst_fds_bios_unload();
    nst_palette_unload();

    UiEmu::stop();
}

bool PNESUiEmu::onInput(c2d::Input::Player *players) {
    return UiEmu::onInput(players);
}

void PNESUiEmu::onUpdate() {
    if (!isPaused()) {
        // update nestopia buttons
        auto *players = getUi()->getInput()->getPlayers();

        for (int i = 0; i < NUMGAMEPADS; i++) {
            cNstPads->pad[i].buttons = 0;

            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Start) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::START : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Select) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::SELECT : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Up) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::UP : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Down) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::DOWN : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Left) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::LEFT : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Right) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::RIGHT : 0;
#ifdef __3DS__
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::A) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::A : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::B) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::B : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::X) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::A : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Y) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::B : 0;
#else
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::A) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::B : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::B) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::A : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::X) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::B : 0;
            cNstPads->pad[i].buttons |= (players[i].buttons & c2d::Input::Button::Y) > 0 ?
                                        Nes::Core::Input::Controllers::Pad::A : 0;
#endif
        }

        uint pointerX = 0;
        uint pointerY = 0;
        bool pointerOnScreen = mapPointerToNesVideo(video, players[0].pointer, pointerX, pointerY);
        bool fire = (players[0].pointer.buttons & c2d::Input::Pointer::Primary) > 0;
        bool hasZapper = Nes::Api::Input(emulator).GetConnectedController(0) == Nes::Api::Input::ZAPPER
                         || Nes::Api::Input(emulator).GetConnectedController(1) == Nes::Api::Input::ZAPPER;
        if (hasZapper) {
            cNstPads->zapper.fire = fire;
            if (pointerOnScreen) {
                cNstPads->zapper.x = pointerX;
                cNstPads->zapper.y = pointerY;
            } else if (fire) {
                cNstPads->zapper.x = ~1U;
            }
        }

        // step nestopia core
        nst_emuloop();

        tickRewind();
    }

    return UiEmu::onUpdate();
}

bool PNESUiEmu::serializeState(std::vector<uint8_t> &out) {
    return nestopia_state_save_memory(out);
}

bool PNESUiEmu::deserializeState(const std::vector<uint8_t> &data) {
    return nestopia_state_load_memory(data);
}

void PNESUiEmu::renderPreviewFrame() {
    // Render a preview frame. Rewind manager restores state after preview.
    nst_emuloop();
    if (video) video->unlock();
}

void PNESUiEmu::clearAudio() {
    // Nestopia resets audio on state load
}

/// NESTOPIA

/// NESTOPIA AUDIO
static void *audio_buffer = nullptr;

void audio_init() {
    int samples = conf.audio_sample_rate / (nst_pal() ? 50 : 60);
    uiEmu->addAudio(conf.audio_sample_rate, samples);
}

void audio_deinit() {
    if (audio_buffer) {
        free(audio_buffer);
        audio_buffer = nullptr;
    }
}

void audio_queue() {
    Audio *audio = uiEmu->getAudio();
    if (audio) {
#if 0
        int samples = uiEmu->getAudio()->getSamples();
        int queued = audio->getSampleBufferQueued();
        int capacity = audio->getSampleBufferCapacity();
        if (samples + queued > capacity) {
            printf("WARNING: samples: %i, queued: %i, capacity: %i (fps: %i)\n",
                   samples, queued, capacity, nst_pal() ? 50 : 60);
        }
#endif
        uiEmu->getAudio()->play(audio_buffer, uiEmu->getAudio()->getSamples(),
                                nst_pal() ? Audio::SyncMode::LowLatency : Audio::SyncMode::None);
    }
}

void audio_pause() {
    if (uiEmu->getAudio()) {
        uiEmu->getAudio()->pause(1);
    }
}

void audio_unpause() {
    if (uiEmu->getAudio()) {
        uiEmu->getAudio()->pause(0);
    }
}

void audio_set_speed(int speed) {
    // TODO
    //bufsize = (channels * (conf.audio_sample_rate / framerate)) / speed;
}

void audio_set_params(Sound::Output *soundoutput) {

    Audio *aud = uiEmu->getAudio();

    if (aud) {
        // Set audio parameters
        Sound sound(emulator);

        sound.SetSampleRate((unsigned long) conf.audio_sample_rate);

        sound.SetSpeaker(Sound::SPEAKER_STEREO);
        sound.SetSpeed(Sound::DEFAULT_SPEED);

        audio_adj_volume();

        audio_buffer = malloc(aud->getSamplesSize());
        memset(audio_buffer, 0, aud->getSamplesSize());
        soundoutput->samples[0] = audio_buffer;
        soundoutput->length[0] = (unsigned int) aud->getSamples();
        soundoutput->samples[1] = nullptr;
        soundoutput->length[1] = 0;
    }
}

void audio_adj_volume() {
    // Adjust the audio volume to the current settings
    Sound sound(emulator);
    sound.SetVolume(Sound::ALL_CHANNELS, conf.audio_volume);
    sound.SetVolume(Sound::CHANNEL_SQUARE1, conf.audio_vol_sq1);
    sound.SetVolume(Sound::CHANNEL_SQUARE2, conf.audio_vol_sq2);
    sound.SetVolume(Sound::CHANNEL_TRIANGLE, conf.audio_vol_tri);
    sound.SetVolume(Sound::CHANNEL_NOISE, conf.audio_vol_noise);
    sound.SetVolume(Sound::CHANNEL_DPCM, conf.audio_vol_dpcm);
    sound.SetVolume(Sound::CHANNEL_FDS, conf.audio_vol_fds);
    sound.SetVolume(Sound::CHANNEL_MMC5, conf.audio_vol_mmc5);
    sound.SetVolume(Sound::CHANNEL_VRC6, conf.audio_vol_vrc6);
    sound.SetVolume(Sound::CHANNEL_VRC7, conf.audio_vol_vrc7);
    sound.SetVolume(Sound::CHANNEL_N163, conf.audio_vol_n163);
    sound.SetVolume(Sound::CHANNEL_S5B, conf.audio_vol_s5b);
}

/// NESTOPIA CONFIG
void PNESUiEmu::nestopia_config_init() {

    // Video
    conf.video_filter = 0;
    conf.video_scale_factor = 1;
    conf.video_palette_mode = 0;
    conf.video_decoder = 0;
    conf.video_brightness = 0; // -100 to 100
    conf.video_saturation = 0; // -100 to 100
    conf.video_contrast = 0; // -100 to 100
    conf.video_hue = 0; // -45 to 45
    conf.video_ntsc_mode = 0;
    conf.video_ntsc_sharpness = 0; // -100 to 100
    conf.video_ntsc_resolution = 0; // -100 to 100
    conf.video_ntsc_bleed = 0; // -100 to 100
    conf.video_ntsc_artifacts = 0; // -100 to 100
    conf.video_ntsc_fringing = 0; // -100 to 100
    conf.video_xbr_corner_rounding = 0;
    conf.video_linear_filter = false;
    conf.video_tv_aspect = false;
    conf.video_unmask_overscan = false;
    conf.video_fullscreen = false;
    conf.video_stretch_aspect = false;
    conf.video_unlimited_sprites = false;
    conf.video_xbr_pixel_blending = false;

    // Audio
    conf.audio_stereo = true;
    conf.audio_sample_rate = 48000;
    conf.audio_volume = 85;
    conf.audio_vol_sq1 = 85;
    conf.audio_vol_sq2 = 85;
    conf.audio_vol_tri = 85;
    conf.audio_vol_noise = 85;
    conf.audio_vol_dpcm = 85;
    conf.audio_vol_fds = 85;
    conf.audio_vol_mmc5 = 85;
    conf.audio_vol_vrc6 = 85;
    conf.audio_vol_vrc7 = 85;
    conf.audio_vol_n163 = 85;
    conf.audio_vol_s5b = 85;

    // Timing
    conf.timing_speed = 60;
    conf.timing_ffspeed = 3;
    conf.timing_turbopulse = 3;

    // Misc
    conf.misc_default_system = 0;
    conf.misc_soft_patching = true;
    conf.misc_genie_distortion = false;
    conf.misc_disable_cursor = false;
    conf.misc_disable_cursor_special = false;
    conf.misc_config_pause = false;
    conf.misc_power_state = 0;
    conf.misc_homebrew_exit = -1;
    conf.misc_homebrew_stdout = -1;
    conf.misc_homebrew_stderr = -1;
}

// NESTOPIA CORE INIT
int PNESUiEmu::nestopia_core_init(const char *rom_path) {

    // Set up directories (nst_set_dirs)
    std::string data_path = pMain->getIo()->getDataPath();
    strncpy(nstpaths.nstdir, data_path.c_str(), sizeof(nstpaths.nstdir));
    strncpy(nstpaths.nstconfdir, data_path.c_str(), sizeof(nstpaths.nstconfdir));
    snprintf(nstpaths.palettepath, sizeof(nstpaths.palettepath), "%s%s", nstpaths.nstdir, "custom.pal");
    // Create directories
    pMain->getIo()->create(data_path);
    pMain->getIo()->create(data_path + "save");
    pMain->getIo()->create(data_path + "state");
    pMain->getIo()->create(data_path + "cheats");
    pMain->getIo()->create(data_path + "screenshots");
    pMain->getIo()->create(data_path + "samples");

    // Set up callbacks
    nst_set_callbacks();

    nst_input_turbo_init();

    // Set archive handler function pointer
    nst_archive_select = &nst_archive_select_file;

    // Set the video dimensions
    video_set_dimensions();

    // Initialize and load FDS BIOS and NstDatabase.xml
    nst_fds_bios_load();
    nst_db_load();

    if (!nst_load(rom_path)) {
        return -1;
    }

    Nes::Api::Input(emulator).AutoSelectControllers();
    Nes::Api::Input(emulator).AutoSelectAdapter();

    nst_video_set_dimensions_screen(nst_video_get_dimensions_screen());

    // Set play in motion
    nst_play();

    return 0;
}

int nestopia_state_load(const char *path) {

    int ret = -1;
    std::filebuf fb;

    if (fb.open(path, std::ios::in)) {
        std::istream is(&fb);
        Machine machine(emulator);
        ret = machine.LoadState(is);
        fb.close();
    }

    return ret;
}

int nestopia_state_save(const char *path) {

    int ret;
    std::ofstream ofs;

    ofs.open(path, std::ofstream::out | std::ofstream::binary);
    Machine machine(emulator);
    ret = machine.SaveState(ofs, Nes::Api::Machine::NO_COMPRESSION);
    ofs.close();

    return ret;
}

void nst_input_turbo_pulse(Nes::Api::Input::Controllers *controllers) {}

void nst_input_turbo_init() {}

void nst_input_init() {}
