//
// Created by Oliver on 04/05/24.
//

#ifndef PNES_MENU_H
#define PNES_MENU_H

class PNESUiMenu : public c2dui::UiMenu {

public:
    explicit PNESUiMenu(UiMain *uiMain);
    virtual void load(bool isRomMenu = false);
    bool onInput(c2d::Input::Player *players) override;
};

#endif  // PNES_MENU_H