//
// Created by Oliver on 04/25/2024
//
#include <string>
#include <iostream>
#include <fstream>

#include "c2dui.h"
#include "uiStateMenu.h"

#include "libretro.h"

PGBAUIStateMenu::PGBAUIStateMenu(c2dui::UiMain *ui) : c2dui::UiStateMenu(ui) {
}

bool PGBAUIStateMenu::loadStateCore(const char *path) {
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    std::streampos fileSize = file.tellg();
    std::string strBuff;
    strBuff.resize(fileSize + 1);

    file.seekg(0, std::ios::beg);
    if (!file.read(const_cast<char*>(strBuff.c_str()), fileSize)) {
        file.close();
        return false;
    }
    file.close();

    if (!retro_unserialize(strBuff.c_str(), fileSize)) {
        return false;
    }

    return true;
}

bool PGBAUIStateMenu::saveStateCore(const char *path) {
    std::string strBuff;
    size_t state_size = retro_serialize_size();

    strBuff.resize(state_size + 1);
    if (!retro_serialize((void *)strBuff.c_str(), state_size)) {
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }

    file.write(strBuff.c_str(), state_size);
    file.close();
    return true;
}
