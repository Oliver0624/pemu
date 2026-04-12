//
// Created by cpasjuste on 01/06/18.
//

#ifndef PSNES_UIEMU_H
#define PSNES_UIEMU_H

#include <string>

class PSNESUiEmu : public c2dui::UiEmu {

public:
    enum class MenuAction {
        None,
        SuperScopeTurbo,
    };

    explicit PSNESUiEmu(c2dui::UiMain *ui);

    int load(const ss_api::Game &game) override;

    void stop();

    void pause() override;

    void resume() override;

    bool onInput(c2d::Input::Player *players) override;

    void onUpdate() override;

    void triggerMenuAction(MenuAction action);

    bool isSuperScopeTurboEnabled() const;

    void setSuperScopeTurboEnabled(bool enabled);

    bool isSuperScopeConnected() const;

    bool isMouseConnected() const;

    bool isJustifierConnected() const;

private:
    MenuAction pendingMenuAction = MenuAction::None;
    bool superScopeTurboEnabled = false;
};

#endif //PSNES_UIEMU_H
