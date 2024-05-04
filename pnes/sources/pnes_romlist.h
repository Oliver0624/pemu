//
// Created by cpasjuste on 02/04/2020.
//

#ifndef PNES_ROMLIST_H
#define PNES_ROMLIST_H

class PNESRomList : public c2dui::RomList {

public:

    PNESRomList(c2dui::UiMain *ui, const std::string &emuVersion, const std::vector<std::string> &filters)
            : c2dui::RomList(ui, emuVersion, filters) {};

    void build(bool addArcadeSystem = false, const ss_api::System &system = {}) override;
};

#endif //PNES_ROMLIST_H
