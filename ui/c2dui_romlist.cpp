//
// Created by cpasjuste on 19/10/16.
//

#include <vector>
#include <string>
#include <algorithm>
#include "c2dui.h"

RomList::RomList(UiMain *_ui, const std::string &emuVersion, const std::vector<std::string> &_filters) {
    printf("RomList()\n");
    ui = _ui;
    filters = _filters;

    // UI
    rect = new C2DRectangle(
            Vector2f(ui->getSize().x - 8, ui->getSize().y - 8));
    ui->getSkin()->loadRectangleShape(rect, {"MAIN"});

    auto *title = new RectangleShape({16, 16});
    if (ui->getSkin()->loadRectangleShape(title, {"MAIN", "TITLE"})) {
        title->setOrigin(Origin::Center);
        title->setPosition(Vector2f(rect->getSize().x / 2, rect->getSize().y / 2));
        float scaling = std::min(
                (rect->getSize().x * 0.8f) / title->getSize().x,
                (rect->getSize().y * 0.8f) / title->getSize().y);
        title->setScale(scaling, scaling);
        rect->add(title);
    } else {
        delete (title);
    }

    text = new Text();
    ui->getSkin()->loadText(text, {"ROM_LIST", "TEXT"});
    text->setOrigin(Origin::BottomLeft);
    text->setPosition(8, rect->getSize().y - 8);
    rect->add(text);

    auto *version = new Text();
    ui->getSkin()->loadText(version, {"ROM_LIST", "TEXT"});
    version->setOrigin(Origin::BottomRight);
    version->setPosition(rect->getSize().x - 8, rect->getSize().y - 8);
    version->setString(emuVersion);
    rect->add(version);

    ui->add(rect);
    ui->flip();
    // UI

    printf("RomList: building list...\n");
    time_start = ui->getElapsedTime().asSeconds();

    gameList = new GameList();
    gameListFav = new GameList();

    printf("RomList()\n");
}

void RomList::setLoadingText(const char *format, ...) {
    char buffer[512];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buffer, 512, format, arg);
    va_end(arg);

    text->setString(buffer);
    ui->flip();
}

void RomList::build(bool addArcadeSystem, const ss_api::System &system) {
    std::string romPath = ui->getConfig()->getRomPath();

    // look for a "gamelist.xml" file inside rom folder, if none found use embedded (romfs) "gamelist.xml"
    std::string gameListPath = romPath + "gamelist.xml";
    if (!ui->getIo()->exist(gameListPath)) {
        gameListPath = ui->getIo()->getRomFsPath() + "gamelist.xml";
    }

    bool showAvailableOnly = ui->getConfig()->get(PEMUConfig::Id::GUI_SHOW_AVAILABLE)->getArrayIndex();
    gameList->append(gameListPath, romPath, false, filters, system, showAvailableOnly);

    setLoadingText("Games: %li / %li", gameList->getAvailableCount(), gameList->games.size());
    printf("RomList::build: %s, games found: %zu / %zu\n",
           gameListPath.c_str(), gameList->getAvailableCount(), gameList->games.size());

    // sort lists
    std::sort(gameList->systemList.systems.begin(), gameList->systemList.systems.end(), Api::sortSystemByName);
    std::sort(gameList->editors.begin(), gameList->editors.end(), Api::sortEditorByName);
    std::sort(gameList->developers.begin(), gameList->developers.end(), Api::sortDeveloperByName);
    std::sort(gameList->genres.begin(), gameList->genres.end(), Api::sortGenreByName);
    std::sort(gameList->players.begin(), gameList->players.end(), Api::sortInteger);
    std::sort(gameList->ratings.begin(), gameList->ratings.end(), Api::sortInteger);
    std::sort(gameList->rotations.begin(), gameList->rotations.end(), Api::sortInteger);
    std::sort(gameList->resolutions.begin(), gameList->resolutions.end(), Api::sortByName);
    std::sort(gameList->dates.begin(), gameList->dates.end(), Api::sortByName);

    gameList->resolutions.insert(gameList->resolutions.begin(), "ALL");
    gameList->dates.insert(gameList->dates.begin(), "ALL");
    if (addArcadeSystem && !gameList->findGamesBySystem(75).empty()) {
        gameList->systemList.systems.insert(gameList->systemList.systems.begin(), {9999, 0, "ARCADE"});
    }

    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_SYSTEM", gameList->systemList.getNames(), 0, PEMUConfig::Id::GUI_FILTER_SYSTEM})->setFlags(
            PEMUConfig::Flags::HIDDEN);
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_GENRE", gameList->getGenreNames(), 0, PEMUConfig::Id::GUI_FILTER_GENRE});
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_DATE", gameList->dates, 0, PEMUConfig::Id::GUI_FILTER_DATE});
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_EDITOR", gameList->getEditorNames(), 0, PEMUConfig::Id::GUI_FILTER_EDITOR});
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_DEVELOPER", gameList->getDeveloperNames(), 0, PEMUConfig::Id::GUI_FILTER_DEVELOPER});
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_PLAYERS", gameList->getPlayersNames(), 0, PEMUConfig::Id::GUI_FILTER_PLAYERS});
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_RATING", gameList->getRatingNames(), 0, PEMUConfig::Id::GUI_FILTER_RATING});
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_ROTATION", gameList->getRotationNames(), 0, PEMUConfig::Id::GUI_FILTER_ROTATION})->setFlags(
            PEMUConfig::Flags::HIDDEN);
    ui->getConfig()->getGroup(PEMUConfig::Id::MENU_MAIN)->addOption(
            {"FILTER_RESOLUTION", gameList->resolutions, 0, PEMUConfig::Id::GUI_FILTER_RESOLUTION})->setFlags(
            PEMUConfig::Flags::HIDDEN);

    // we need to reload config to update new options we just added
    ui->getConfig()->load();

    gameListFav->append(ui->getIo()->getDataPath() + "favorites.xml");
    for (size_t i = 0; i < gameListFav->games.size(); i++) {
        Game game = gameList->findGameByPathAndSystem(gameListFav->games[i].path, gameListFav->games[i].system.id);
        if (!game.path.empty()) {
            gameListFav->games[i].available = true;
            gameListFav->games[i].romsPath = game.romsPath;
        }
    }
    printf("RomList::build: %zu favorites\n", gameListFav->games.size());

    float time_spent = ui->getElapsedTime().asSeconds() - time_start;
    printf("RomList::build(): list built in %f\n", time_spent);

    // UI
    // remove ui widgets
    delete (rect);
}

void RomList::addFav(const Game &game) {
    if (!gameListFav->exist(game.id)) {
        gameListFav->games.emplace_back(game);
        gameListFav->save(ui->getIo()->getDataPath() + "favorites.xml", "mixrbv2", "", "video");
    }
}

void RomList::removeFav(const Game &game) {
    if (gameListFav->remove(game.id)) {
        gameListFav->save(ui->getIo()->getDataPath() + "favorites.xml", "mixrbv2", "", "video");
    }
}

RomList::~RomList() {
    printf("~RomList()\n");
    delete (gameList);
    delete (gameListFav);
}
