//
// Created by Oliver on 04/25/2024
//

#include "c2dui.h"
#include "uiEmu.h"
#include "uiStateMenu.h"
#include "pgba_config.h"
#include "pgba_romlist.h"
#include "pgba_io.h"
#include "pgba_core.h"

using namespace c2d;
using namespace c2dui;

#ifdef __PSP2__
#include <psp2/power.h>
#include <psp2/io/dirent.h>
#define mkdir(x, y) sceIoMkdir(x, 0777)
#elif __PS4__
extern "C" int sceSystemServiceLoadExec(const char *path, const char *args[]);
#endif

UiMenu *uiMenu;
PGBAUiEmu *uiEmu;
PGBAConfig *cfg;
PGBAUIStateMenu *uiState;
PGBARomList *romList;

UiMain *ui;
Skin *skin;
UIRomList *uiRomList;

int main(int argc, char **argv) {
#ifdef __PSP2__
    // set max cpu speed
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
#endif

    // need custom io for some devices
    auto *io = new PGBAIo();
    // create paths
    io->create(io->getDataPath());
    io->create(io->getDataPath() + "roms");
    io->create(io->getDataPath() + "configs");
    io->create(io->getDataPath() + "saves");
    io->create(io->getDataPath() + "save");

    // create main ui
    ui = new UiMain(io);

    // load custom configuration
    int pgba_version = (__PGBA_VERSION_MAJOR__ * 100) + __PGBA_VERSION_MINOR__;
    cfg = new PGBAConfig(ui, pgba_version);
    ui->setConfig(cfg);

    // load skin configuration
    skin = new Skin(ui);
    ui->setSkin(skin);

    // ui
    std::string mgba_version = "mGBA " MGBA_VERSION ;
    romList = new PGBARomList(ui, mgba_version, {".zip", ".7z", ".gba", ".gbc", ".gb", ".sgb"});
    romList->build();
    uiRomList = new UIRomList(ui, romList, ui->getSize(), UIRomList::Pnes);
    uiMenu = new UiMenu(ui);
    uiEmu = new PGBAUiEmu(ui);
    uiState = new PGBAUIStateMenu(ui);
    ui->init(uiRomList, uiMenu, uiEmu, uiState);

    while (!ui->done) {
        ui->flip();
    }

    delete (skin);
    delete (cfg);
    delete (ui);

#ifdef __PSP2__
    scePowerSetArmClockFrequency(266);
    scePowerSetBusClockFrequency(166);
    scePowerSetGpuClockFrequency(166);
    scePowerSetGpuXbarClockFrequency(111);
#elif __PS4__
    sceSystemServiceLoadExec((char *) "exit", nullptr);
    while (true) {}
#endif

    return 0;
}
