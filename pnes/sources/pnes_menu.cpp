//
// Created by Oliver on 04/05/24.
//
#include "fltkui/nstcommon.h"

#include "c2dui.h"
#include "c2dui_ui_menu.h"
#include "pnes_menu.h"

#define OPTION_ID_QUIT (-1)
#define OPTION_ID_STATES (-2)
#define OPTION_ID_OTHER (-3)

PNESUiMenu::PNESUiMenu(UiMain *uiMain) : c2dui::UiMenu(uiMain) {
    printf("PNESUiMenu()\n");
}


void PNESUiMenu::load(bool isRom) {

    isRomMenu = isRom;
    isEmuRunning = ui->getUiEmu()->isVisible();
    Game game = ui->getUiRomList()->getSelection();

    if (isRomMenu) {
        ui->getConfig()->load(game);
        title->setString(game.name);
    } else {
        title->setString(TEXT_TITTLE_MAIN_OPTIONS);
    }

    // set options items (remove hidden options)
    optionIndex = highlightIndex = 0;
    options.clear();
    std::vector<Option> *opts = ui->getConfig()->get(isRomMenu);
    remove_copy_if(opts->begin(), opts->end(),
                   back_inserter(options), [this](Option &opt) {
                return isOptionHidden(&opt) || opt.getFlags() & Option::Flags::HIDDEN;
            });

    options.push_back({TEXT_OTHER, {}, 0, OPTION_ID_OTHER, Option::Flags::MENU});
    if (isRomMenu) {
        options.push_back({TEXT_PNES_DISK_FLIP, {std::make_pair("GO",TEXT_GO)}, 0, Option::Id::ROM_PNES_DISK_FLIP, Option::Flags::STRING});
        options.push_back({TEXT_STATES, {std::make_pair("GO",TEXT_GO)}, 0, OPTION_ID_STATES, Option::Flags::STRING});
    }
    options.push_back({TEXT_QUIT, {std::make_pair("GO",TEXT_GO)}, 0, OPTION_ID_QUIT, Option::Flags::STRING});

    setAlpha(isEmuRunning ? (uint8_t) (alpha - 50) : (uint8_t) alpha);

    // update options lines/items (first option should always be a menu option)
    onKeyDown();

    // finally, show me
    if (!isVisible()) {
        setLayer(1);
        setVisibility(Visibility::Visible, true);
    }
}

bool PNESUiMenu::onInput(c2d::Input::Player *players) {
    unsigned int buttons = players[0].buttons;

    if (ui->getUiStateMenu()->isVisible()) {
        return C2DObject::onInput(players);
    }

    if (buttons & c2d::Input::Button::A) {
        Option option = lines.at(highlightIndex)->option;
        if (option.getId() == Option::Id::ROM_PNES_DISK_FLIP) {
            nst_fds_flip();
            setVisibility(Visibility::Hidden, true);
            ui->getUiEmu()->resume();
            return true;
        }
    }

    return UiMenu::onInput(players);
}