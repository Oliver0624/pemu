//
// Created by cpasjuste on 29/05/18.
//

#include "c2dui.h"
#include "config.h"

using namespace c2d;
using namespace c2dui;
//OLIVER
PSNESConfig::PSNESConfig(UiMain *ui, int version) : Config(ui, version) {
    printf("PSNESConfig(%s, v%i)\n", getConfigPath().c_str(), version);

    add(Option::Id::ROM_SHOW_FPS, "AUDIO_SYNC", TEXT_MENU_AUDIO_SYNC, {"OFF", "ON"}, 0,
        Option::Id::ROM_AUDIO_SYNC, Option::Flags::BOOLEAN);
    get(Option::Id::ROM_AUDIO_SYNC)->setInfo("ON: PERFECT AUDIO - OFF: MINOR AUDIO STUTTERING (FAVOR FPS)");

    add(Option::Id::ROM_AUDIO_SYNC, "CHEATS", TEXT_MENU_CHEATS, {"OFF", "ON"}, 1,
        Option::Id::ROM_PSNES_CHEATS, Option::Flags::BOOLEAN);

    add(Option::Id::ROM_PSNES_CHEATS, "BLOCK_INVALID_VRAM", TEXT_MENU_BLOCK_INVALID_VRAM, {"OFF", "ON"}, 1,
        Option::Id::ROM_PSNES_BLOCK_VRAM, Option::Flags::BOOLEAN);

    add(Option::Id::ROM_PSNES_BLOCK_VRAM, "TRANSPARENCY", TEXT_MENU_TRANSPARENCY, {"OFF", "ON"}, 1,
        Option::Id::ROM_PSNES_TRANSPARENCY, Option::Flags::BOOLEAN);

    add(Option::Id::ROM_PSNES_TRANSPARENCY, "DISPLAY_MESSAGES", TEXT_MENU_DISPLAY_MESSAGES, {"OFF", "ON"}, 1,
        Option::Id::ROM_PSNES_DISPLAY_MESSAGES, Option::Flags::BOOLEAN);

    add(Option::Id::ROM_PSNES_DISPLAY_MESSAGES, "FRAMESKIP", TEXT_MENU_FRAMESKIP,
        {"OFF", "AUTO", "1", "2", "3", "4", "5", "6", "7", "8", "9"}, 0,
        Option::Id::ROM_PSNES_FRAMESKIP, Option::Flags::STRING);
#ifdef __VITA__
    get(Option::Id::ROM_PSNES_FRAMESKIP)->setValueString("2");
#endif

    add(Option::Id::ROM_PSNES_FRAMESKIP, "TURBO_MODE", TEXT_MENU_TURBO_MODE, {"OFF", "ON"}, 0,
        Option::Id::ROM_PSNES_TURBO_MODE, Option::Flags::BOOLEAN);

    add(Option::Id::ROM_PSNES_TURBO_MODE, "TURBO_FRAMESKIP", TEXT_MENU_TURBO_FRAMESKIP,
        {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
         "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25"}, 15,
        Option::Id::ROM_PSNES_TURBO_FRAMESKIP, Option::Flags::STRING);

    // no need for auto-scaling mode on psnes
    get(Option::Id::ROM_SCALING_MODE)->set( // OLIVER TODO
            {"SCALING_MODE", {"ASPECT", "INTEGER"}, 0,
             Option::Id::ROM_SCALING_MODE, Option::Flags::STRING});

    // "c2dui_romlist" will also reload config, but we need new roms paths
    reset();
    load();
}
