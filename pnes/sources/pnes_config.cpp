//
// Created by cpasjuste on 29/05/18.
//

#include "c2dui.h"
#include "pnes_config.h"

using namespace c2d;
using namespace c2dui;

PNESConfig::PNESConfig(UiMain *ui, int version) : Config(ui, version) {
    roms_paths.clear();
    roms_paths.emplace_back(ui->getIo()->getDataPath() + "roms/nes/");
    roms_paths.emplace_back(ui->getIo()->getDataPath() + "roms/fds/");

    // no need for auto-scaling mode on pnes
    get(Option::Id::ROM_SCALING_MODE)->set(
            {"SCALING_MODE", {std::make_pair("ASPECT",TEXT_OPTION_ASPECT), std::make_pair("INTEGER",TEXT_OPTION_INTEGER)}, 0,
             Option::Id::ROM_SCALING_MODE, Option::Flags::STRING});

#ifdef __SWITCH__
    // on nintendo switch invert A/B buttons
    get(Option::Id::JOY_A)->setValueInt(KEY_JOY_B_DEFAULT);
    get(Option::Id::JOY_B)->setValueInt(KEY_JOY_A_DEFAULT);
#endif

    // "c2dui_romlist" will also reload config, but we need new roms paths
    reset();
    load();
}
