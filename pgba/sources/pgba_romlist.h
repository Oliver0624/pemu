//
// Created by Oliver on 04/05/2024.
//

#ifndef PGBA_ROMLIST_H
#define PGBA_ROMLIST_H

class PGBARomList : public c2dui::RomList {

public:

    PGBARomList(c2dui::UiMain *ui, const std::string &emuVersion, const std::vector<std::string> &filters)
            : c2dui::RomList(ui, emuVersion, filters) {};

    void build(bool addArcadeSystem = false, const ss_api::System &system = {}, bool skipAppend = false) override;
};

#endif //PGBA_ROMLIST_H
