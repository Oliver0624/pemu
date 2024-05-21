//
// Created by Oliver on 04/05/2024.
//

#include "c2dui.h"
#include "pgba_romlist.h"

void PGBARomList::build(bool addArcadeSystem, const ss_api::System &system, bool skipAppend) {
    std::vector<std::string> gameLists = {
            "gamelist_gb.xml",
            "gamelist_gbc.xml",
            "gamelist_gba.xml"
    };

    bool showAvailableOnly = ui->getConfig()->get(Option::Id::GUI_SHOW_AVAILABLE)->getValueBool();

    for (size_t i = 0; i < gameLists.size(); i++) {
        // look for a "gamelist.xml" file inside rom folder, if none found use embedded (romfs) "gamelist.xml"
        std::string gameListPath = ui->getConfig()->getRomPaths().at(i) + "gamelist.xml";
        if (!ui->getIo()->exist(gameListPath)) {
            gameListPath = ui->getIo()->getRomFsPath() + gameLists.at(i);
            if (!ui->getIo()->exist(gameListPath)) continue;
        }
        gameList->append(gameListPath, ui->getConfig()->getRomPaths().at(i),
                         false, filters, system, showAvailableOnly);
        setLoadingText(TEXT_MSG_GAMES_SEARCH, gameList->getAvailableCount(), gameList->games.size());
        printf("RomList::build: %s, games found: %zu / %zu\n",
               gameListPath.c_str(), gameList->getAvailableCount(), gameList->games.size());
    }

    RomList::build(addArcadeSystem, {}, true);

    // enable system filtering
    ui->getConfig()->get(Option::Id::GUI_FILTER_SYSTEM)->setFlags(Option::Flags::STRING);
}
