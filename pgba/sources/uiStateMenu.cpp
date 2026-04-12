//
// Created by Oliver on 04/25/2024
//

#include "uiStateMenu.h"

extern "C" {
#include "mgba-util/vfs.h"
#include "mgba/core/core.h"
#include "mgba/core/serialize.h"
}

extern mCore *s_core;

PGBAUIStateMenu::PGBAUIStateMenu(c2dui::UiMain *ui) : c2dui::UiStateMenu(ui) {
}

bool PGBAUIStateMenu::loadStateCore(const char *path) {
    auto *vf = VFileOpen(path, O_RDONLY);
    if (!vf || !s_core) {
        return false;
    }

    bool success = mCoreLoadStateNamed(s_core, vf, SAVESTATE_RTC);
    vf->close(vf);
    return success;
}

bool PGBAUIStateMenu::saveStateCore(const char *path) {
    auto *vf = VFileOpen(path, O_CREAT | O_TRUNC | O_RDWR);
    if (!vf || !s_core) {
        return false;
    }

    bool success = mCoreSaveStateNamed(s_core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC | SAVESTATE_METADATA);
    vf->close(vf);

    if (!success) {
        getUi()->getIo()->removeFile(path);
    }

    return success;
}
