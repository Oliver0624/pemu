//
// Created by Oliver on 04/25/2024
//

#include <fstream>

#include "c2dui.h"
#include "uiEmu.h"

#include "pgba_core.h"
#include "libretro.h"

extern PGBAUiEmu *uiEmu;

PGBAUiEmu::PGBAUiEmu(UiMain *ui) : UiEmu(ui) {
    printf("PGBAGuiEmu()\n");
    mgba_init();
}

PGBAUiEmu::~PGBAUiEmu() {
    printf("~PGBAGuiEmu()\n");
    mgba_fini();
}

int PGBAUiEmu::load(const ss_api::Game &game) {
    getUi()->getUiProgressBox()->setTitle(game.name);
    getUi()->getUiProgressBox()->setMessage(TEXT_MSG_PLEASE_WAIT);
    getUi()->getUiProgressBox()->setProgress(0);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Visible);
    getUi()->getUiProgressBox()->setLayer(1000);
    getUi()->flip();

    if (0 != mgba_start(game)) {
        return -1;
    }

    getUi()->getUiProgressBox()->setProgress(1);
    getUi()->flip();
    getUi()->delay(500);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);

    return UiEmu::load(game);
}

void PGBAUiEmu::stop() {
    mgba_stop();
    UiEmu::stop();
}

bool PGBAUiEmu::onInput(c2d::Input::Player *players) {
    return UiEmu::onInput(players);
}

void PGBAUiEmu::onUpdate() {
    if (!isPaused()) {
        mgbp_step();
    }

    return UiEmu::onUpdate();
}