//
// Created by Oliver on 04/25/2024
//

#ifndef PGBA_UIEMU_H
#define PGBA_UIEMU_H

#include <string>

class PGBAUiEmu : public c2dui::UiEmu {
public:
    explicit PGBAUiEmu(c2dui::UiMain *ui);
    virtual ~PGBAUiEmu();

    int  load(const ss_api::Game &game) override;
    void stop() override;
    bool onInput(c2d::Input::Player *players) override;
    void onUpdate() override;
};

#endif //PGBA_UIEMU_H
