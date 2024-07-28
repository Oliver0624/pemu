//
// Created by cpasjuste on 02/04/2020.
//

#include "c2dui.h"
#include "pgen_romlist.h"

void PGENRomList::build(bool addArcadeSystem, const ss_api::System &system, bool skipAppend) {
    std::vector<std::string> gameLists = {
            "gamelist_genesis.xml",
            "gamelist_megacd.xml",
            "gamelist_sms.xml",
            "gamelist_gamegear.xml"
#if 0
            "gamelist_sg1000.xml",
#endif
    };

    std::vector<ss_api::System> systemLists = {
        {1,  0, "Genesis"},
        {20, 1, "Mega-CD"},
        {2,  0, "Master System"},
        {21, 0, "Game Gear"}
#if 0
        {109, 2, "SG-1000"},
#endif
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
                         false, filters, systemLists[i], showAvailableOnly);
        setLoadingText(TEXT_MSG_GAMES_SEARCH, gameList->getAvailableCount(), gameList->games.size());
        printf("RomList::build: %s, games found: %zu / %zu, path : %s\n",
               gameListPath.c_str(), gameList->getAvailableCount(), gameList->games.size(),
               ui->getConfig()->getRomPaths().at(i).c_str());
    }

    RomList::build(addArcadeSystem, {}, true);

    // enable system filtering
    ui->getConfig()->get(Option::Id::GUI_FILTER_SYSTEM)->setFlags(Option::Flags::STRING);
}
