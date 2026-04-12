//
// Created by OpenAI Codex on 12/04/26.
//

#ifndef PSNES_UIMENU_H
#define PSNES_UIMENU_H

#include "c2dui_ui_menu.h"

class PSNESUiMenu : public c2dui::UiMenu {

public:
    explicit PSNESUiMenu(c2dui::UiMain *uiMain);

    void load(bool isRom = false) override;

    bool onInput(c2d::Input::Player *players) override;
};

#endif //PSNES_UIMENU_H
