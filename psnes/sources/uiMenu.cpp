//
// Created by OpenAI Codex on 12/04/26.
//

#include "c2dui.h"
#include "uiMenu.h"
#include "uiEmu.h"

#define OPTION_ID_QUIT (-1)
#define OPTION_ID_STATES (-2)
#define OPTION_ID_OTHER (-3)
#define OPTION_ID_SUPERSCOPE_TURBO (-100)

using namespace c2d;
using namespace c2dui;

PSNESUiMenu::PSNESUiMenu(UiMain *uiMain) : UiMenu(uiMain) {
    printf("PSNESUiMenu()\n");
}

void PSNESUiMenu::load(bool isRom) {
    isRomMenu = isRom;
    isEmuRunning = ui->getUiEmu()->isVisible();
    Game game = ui->getUiRomList()->getSelection();

    if (isRomMenu) {
        ui->getConfig()->load(game);
        title->setString(game.name);
    } else {
        title->setString(TEXT_TITTLE_MAIN_OPTIONS);
    }

    optionIndex = highlightIndex = 0;
    options.clear();
    std::vector<Option> *opts = ui->getConfig()->get(isRomMenu);
    remove_copy_if(opts->begin(), opts->end(),
                   back_inserter(options), [this](Option &opt) {
                return isOptionHidden(&opt) || opt.getFlags() & Option::Flags::HIDDEN;
            });

    options.push_back({TEXT_OTHER, {}, 0, OPTION_ID_OTHER, Option::Flags::MENU});

    if (isRomMenu) {
        auto *emu = dynamic_cast<PSNESUiEmu *>(ui->getUiEmu());
        if (emu && emu->isSuperScopeConnected()) {
            options.push_back({TEXT_PSNES_SUPERSCOPE_TURBO,
                               {std::make_pair("0", TEXT_OPTION_OFF),
                                std::make_pair("1", TEXT_OPTION_ON)},
                               emu->isSuperScopeTurboEnabled() ? 1 : 0,
                               OPTION_ID_SUPERSCOPE_TURBO,
                               Option::Flags::BOOLEAN});
        }
        options.push_back({TEXT_STATES, {std::make_pair("GO", TEXT_GO)}, 0, OPTION_ID_STATES,
                           Option::Flags::STRING});
    }

    options.push_back({TEXT_QUIT, {std::make_pair("GO", TEXT_GO)}, 0, OPTION_ID_QUIT,
                       Option::Flags::STRING});

    setAlpha(isEmuRunning ? (uint8_t) (alpha - 50) : (uint8_t) alpha);
    onKeyDown();

    if (!isVisible()) {
        setLayer(1);
        setVisibility(Visibility::Visible, true);
    }
}

bool PSNESUiMenu::onInput(c2d::Input::Player *players) {
    unsigned int buttons = players[0].buttons;

    if (ui->getUiStateMenu()->isVisible()) {
        return C2DObject::onInput(players);
    }

    auto *emu = dynamic_cast<PSNESUiEmu *>(ui->getUiEmu());
    Option option = lines.at(highlightIndex)->option;

    if (emu && option.getId() == OPTION_ID_SUPERSCOPE_TURBO &&
        (buttons & Input::Button::Left || buttons & Input::Button::Right)) {
        bool enabled = (buttons & Input::Button::Right) != 0;
        option.setValueBool(enabled);
        options.at(optionIndex + highlightIndex).set(option);
        lines.at(highlightIndex)->set(option);
        emu->setSuperScopeTurboEnabled(enabled);
        ui->getUiStatusBox()->show("%s : %s",
                                   option.getDisplayName().c_str(),
                                   option.getValueDisplayString().c_str());
        return true;
    }

    if (buttons & Input::Button::A) {
        if (emu && option.getId() == OPTION_ID_SUPERSCOPE_TURBO) {
            bool enabled = !option.getValueBool();
            option.setValueBool(enabled);
            options.at(optionIndex + highlightIndex).set(option);
            lines.at(highlightIndex)->set(option);
            emu->setSuperScopeTurboEnabled(enabled);
            ui->getUiStatusBox()->show("%s : %s",
                                       option.getDisplayName().c_str(),
                                       option.getValueDisplayString().c_str());
            return true;
        }
    }

    return UiMenu::onInput(players);
}
