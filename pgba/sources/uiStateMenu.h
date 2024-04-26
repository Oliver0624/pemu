//
// Created by Oliver on 04/25/2024
//

#ifndef PEMU_PGBAUISTATEMENU_H
#define PEMU_PGBAUISTATEMENU_H

#include "c2dui_ui_main.h"
#include "c2dui_ui_menu_state.h"

class PGBAUIStateMenu : public c2dui::UiStateMenu {

public:
    PGBAUIStateMenu(c2dui::UiMain *ui);

    bool loadStateCore(const char *path);

    bool saveStateCore(const char *path);

};

#endif //PEMU_PGBAUISTATEMENU_H
