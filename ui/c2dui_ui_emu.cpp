//
// Created by cpasjuste on 03/02/18.
//

#include "c2dui.h"

UiEmu::UiEmu(UiMain *u) : RectangleShape(u->getSize()) {
    printf("UiEmu()\n");

    pMain = u;
    RectangleShape::setFillColor(Color::Transparent);

    fpsText = new Text("0123456789", (unsigned int) pMain->getFontSize(), pMain->getSkin()->getFont());
    fpsText->setFillColor(Color::Yellow);
    fpsText->setOutlineColor(Color::Black);
    fpsText->setOutlineThickness(1);
    fpsText->setString("FPS: 00/60");
    fpsText->setPosition(16, 16);
    fpsText->setVisibility(Visibility::Hidden);
    RectangleShape::add(fpsText);

    // Create rewind panel and texts
    float panelHeight = 118.0f * pMain->getScaling().y;
    rewindPanel = new RectangleShape({pMain->getSize().x, panelHeight});
    rewindPanel->setFillColor(Color(0, 0, 0, 180));
    rewindPanel->setOutlineColor(Color(255, 255, 255, 90));
    rewindPanel->setOutlineThickness(1);
    rewindPanel->setPosition(0, pMain->getSize().y - panelHeight);
    rewindPanel->setLayer(90);
    rewindPanel->setVisibility(Visibility::Hidden);
    RectangleShape::add(rewindPanel);

    rewindTitleText = new Text("", (unsigned int) (20 * pMain->getSkin()->getFontScaling()),
                               pMain->getSkin()->getFont());
    rewindTitleText->setFillColor(Color::White);
    rewindTitleText->setOutlineColor(Color::Black);
    rewindTitleText->setOutlineThickness(2);
    rewindTitleText->setPosition(16, rewindPanel->getPosition().y + (10 * pMain->getScaling().y));
    rewindTitleText->setLayer(100);
    rewindTitleText->setVisibility(Visibility::Hidden);
    RectangleShape::add(rewindTitleText);

    rewindTimelineText = new Text("", (unsigned int) (22 * pMain->getSkin()->getFontScaling()),
                                  pMain->getSkin()->getFont());
    rewindTimelineText->setFillColor(Color::White);
    rewindTimelineText->setOutlineColor(Color::Black);
    rewindTimelineText->setOutlineThickness(2);
    rewindTimelineText->setLayer(100);
    rewindTimelineText->setVisibility(Visibility::Hidden);
    RectangleShape::add(rewindTimelineText);

    rewindHintText = new Text("", (unsigned int) (18 * pMain->getSkin()->getFontScaling()),
                              pMain->getSkin()->getFont());
    rewindHintText->setFillColor(Color::White);
    rewindHintText->setOutlineColor(Color::Black);
    rewindHintText->setOutlineThickness(2);
    rewindHintText->setPosition(16, rewindPanel->getPosition().y + (84 * pMain->getScaling().y));
    rewindHintText->setLayer(100);
    rewindHintText->setVisibility(Visibility::Hidden);
    RectangleShape::add(rewindHintText);

    RectangleShape::setVisibility(Visibility::Hidden);
}

void UiEmu::addAudio(c2d::Audio *_audio) {
    delete (audio);
    audio = _audio;
}

void UiEmu::addAudio(int rate, int samples, Audio::C2DAudioCallback cb) {
    auto *aud = new C2DAudio(rate, samples, cb);
    addAudio(aud);
}

void UiEmu::addVideo(C2DUIVideo *v) {
    delete (video);
    video = v;
    video->setShader(pMain->getConfig()->get(Option::Id::ROM_SHADER, true)->getIndex());
    video->setFilter((Texture::Filter) pMain->getConfig()->get(Option::Id::ROM_FILTER, true)->getIndex());
    video->updateScaling();
    add(video);
}

void UiEmu::addVideo(uint8_t **pixels, int *pitch,
                     const c2d::Vector2i &size, const c2d::Vector2i &aspect, Texture::Format format) {
    auto *v = new C2DUIVideo(pMain, pixels, pitch, size, aspect, format);
    addVideo(v);
}

int UiEmu::load(const Game &game) {
    printf("UiEmu::load: name: %s, path: %s\n",
           game.path.c_str(), game.romsPath.c_str());
    pMain->getUiStatusBox()->show(TEXT_MSG_PRESS_MENU1_2_FOR_INGAME_MENU);
    currentGame = game;

    // set fps text on top
    getFpsText()->setLayer(2);

    setVisibility(Visibility::Visible);
    pMain->getUiProgressBox()->setVisibility(Visibility::Hidden);
    pMain->getUiRomList()->setVisibility(Visibility::Hidden);
    pMain->getUiRomList()->getBlur()->setVisibility(Visibility::Hidden);

    promptLoadAutoState();
    resume();

    return 0;
}

void UiEmu::pause() {
    printf("UiEmu::pause()\n");
    if (audio != nullptr) {
        audio->pause(1);
    }
    // set ui input configuration
    pMain->updateInputMapping(false);
    // enable auto repeat delay
    pMain->getInput()->setRepeatDelay(INPUT_DELAY);

    paused = true;
}

void UiEmu::resume() {
    printf("UiEmu::resume()\n");
    // set per rom input configuration
    pMain->updateInputMapping(true);
    // disable auto repeat delay
    pMain->getInput()->setRepeatDelay(0);

#ifdef __VITA__
    Option *option = pMain->getConfig()->get(Option::Id::ROM_WAIT_RENDERING, true);
    if (option) {
        ((PSP2Renderer *) pMain)->setWaitRendering(option->getValueBool());
    }
#endif

    if (audio) {
        // Keep audio muted if rewind fade is active; otherwise full volume.
        audio->setGain(rewindAudioFadeActive ? 0.0f : 1.0f);
        audio->pause(0);
    }

    paused = false;
}

void UiEmu::stop() {
    printf("UiEmu::stop()\n");
    if (audio) {
        printf("Closing audio...\n");
        audio->setGain(1.0f);
        audio->pause(1);
        delete (audio);
        audio = nullptr;
    }

    if (video) {
        printf("Closing video...\n");
        delete (video);
        video = nullptr;
    }

#ifdef __VITA__
    ((PSP2Renderer *) pMain)->setWaitRendering(true);
#endif

    pMain->updateInputMapping(false);
    setVisibility(Visibility::Hidden);
    rewindOverlayVisiblePrev = false;
    rewindAudioFadeActive = false;
}

bool UiEmu::onInput(c2d::Input::Player *players) {
    if (pMain->getUiMenu()->isVisible() || pMain->getUiStateMenu()->isVisible()) {
        return C2DObject::onInput(players);
    }

    // look for player 1 menu combo
    if (((players[0].buttons & Input::Button::Menu1) && (players[0].buttons & Input::Button::Menu2))) {
        pause();
        pMain->getUiMenu()->load(true);
        pMain->getInput()->clear();
        return true;
    }

    // Handle rewind input
    if (supportsRewind()) {
        unsigned int buttons = players[0].buttons;
        bool lb = (buttons & Input::Button::LB) != 0;
        bool rb = (buttons & Input::Button::RB) != 0;
        bool left = (buttons & Input::Button::Left) != 0;
        bool right = (buttons & Input::Button::Right) != 0;
        bool a = (buttons & Input::Button::A) != 0;
        bool b = (buttons & Input::Button::B) != 0;

        auto updateHint = [](void *userData, const char * /*text*/) -> bool {
            UiEmu *self = static_cast<UiEmu *>(userData);
            self->updateRewindOverlay();
            return true;
        };

        bool handled = rewindManager.handleInput(
                lb, rb, left, right, a, b,
                [this](std::vector<uint8_t> &data) { return serializeState(data); },
                [this](const std::vector<uint8_t> &data) { return deserializeState(data); },
                [this]() { pMain->getInput()->clear(); },
                [this]() { pause(); },
                [this]() { resume(); },
                [this]() { previewRewind(); },
                [this]() { clearAudio(); },
                updateHint, this);

        if (handled) {
            updateRewindOverlay();
            return true;
        }
    }

    return C2DObject::onInput(players);
}

void UiEmu::onUpdate() {
    C2DObject::onUpdate();

    if (!isPaused()) {
        // fps
        bool showFps = pMain->getConfig()->get(Option::Id::ROM_SHOW_FPS, true)->getValueBool();
        if (showFps) {
            if (!fpsText->isVisible()) {
                fpsText->setVisibility(c2d::Visibility::Visible);
            }
            sprintf(fpsString, "FPS: %.1f/%.1f", pMain->getFps(), targetFps);
            fpsText->setString(fpsString);
        } else {
            if (fpsText->isVisible()) {
                fpsText->setVisibility(c2d::Visibility::Hidden);
            }
        }
    }

    if (supportsRewind()) {
        updateRewindOverlay();
        updateRewindAudioFade();
    }
}

UiMain *UiEmu::getUi() {
    return pMain;
}

C2DUIVideo *UiEmu::getVideo() {
    return video;
}

c2d::Audio *UiEmu::getAudio() {
    return audio;
}

c2d::Text *UiEmu::getFpsText() {
    return fpsText;
}

bool UiEmu::isPaused() {
    return paused;
}

std::string UiEmu::getAutoStatePath(const Game &game) const {
    if (game.path.empty()) {
        return "";
    }

    return pMain->getIo()->getDataPath() + "saves/" + Utility::removeExt(game.path) + ".auto.sav";
}

bool UiEmu::promptLoadAutoState() {
    std::string path = getAutoStatePath(currentGame);
    if (path.empty() || !pMain->getIo()->exist(path)) {
        return false;
    }

    int res = pMain->getUiMessageBox()->show(
            currentGame.name,
            TEXT_MSG_AUTO_SAVE_FOUND,
            TEXT_BUTTON_LOAD,
            TEXT_BUTTON_CANCEL);
    if (res == MessageBox::LEFT) {
        return pMain->getUiStateMenu()->loadStateCore(path.c_str());
    }

    return false;
}

bool UiEmu::saveAutoState() {
    std::string path = getAutoStatePath(currentGame);
    if (path.empty()) {
        return false;
    }

    return pMain->getUiStateMenu()->saveStateCore(path.c_str());
}

void UiEmu::initRewindUi() {
    if (!supportsRewind()) {
        return;
    }
    rewindManager.init((int) targetFps, 128);
}

void UiEmu::tickRewind() {
    if (!supportsRewind()) {
        return;
    }
    // Always capture snapshots during normal gameplay
    if (!rewindManager.isTimelineVisible()) {
        rewindManager.tickAndCapture([this](std::vector<uint8_t> &data) { return serializeState(data); });
    }
}

void UiEmu::resetRewind() {
    rewindManager.reset();
    updateRewindOverlay();
}

void UiEmu::previewRewind() {
    // Refresh current video buffer without advancing emulation.
    renderPreviewFrame();
}

void UiEmu::updateRewindOverlay() {
    bool visible = supportsRewind() && rewindManager.isTimelineVisible();
    rewindPanel->setVisibility(visible ? Visibility::Visible : Visibility::Hidden);
    rewindTitleText->setVisibility(visible ? Visibility::Visible : Visibility::Hidden);
    rewindTimelineText->setVisibility(visible ? Visibility::Visible : Visibility::Hidden);
    rewindHintText->setVisibility(visible ? Visibility::Visible : Visibility::Hidden);

    if (!visible) {
        if (rewindOverlayVisiblePrev) {
            // Leaving rewind mode: fade audio in to avoid abrupt pop/click.
            if (audio && !audio->isPaused()) {
                audio->setGain(0.0f);
                rewindAudioFadeActive = true;
                rewindAudioFadeStart = std::chrono::steady_clock::now();
            }
        }
        rewindOverlayVisiblePrev = false;
        return;
    }

    // Entering rewind mode: keep output muted while browsing timeline.
    if (!rewindOverlayVisiblePrev && audio) {
        audio->setGain(0.0f);
        rewindAudioFadeActive = false;
    }
    rewindOverlayVisiblePrev = true;

    int selected = rewindManager.getSelectionSecondsAgo();
    int total = rewindManager.getHistorySeconds();
    char titleBuf[128];
    std::snprintf(titleBuf, sizeof(titleBuf), "%s  %02ds / %02ds",
                  TEXT_REWIND_TITLE, selected, total);
    rewindTitleText->setString(titleBuf);

    std::string slots = rewindManager.getTimelineSlotsText(7);
    rewindTimelineText->setString(slots.empty() ? TEXT_REWIND_EMPTY : slots);
    auto timelineBounds = rewindTimelineText->getLocalBounds();
    float timelineX = (pMain->getSize().x - timelineBounds.width) / 2.0f;
    float timelineY = rewindPanel->getPosition().y + (44 * pMain->getScaling().y);
    rewindTimelineText->setPosition(timelineX, timelineY);

    rewindHintText->setString(TEXT_REWIND_HINT);
}

void UiEmu::updateRewindAudioFade() {
    if (!rewindAudioFadeActive || !audio || audio->isPaused()) {
        return;
    }

    constexpr float fadeDurationSec = 0.20f;
    auto now = std::chrono::steady_clock::now();
    float elapsedSec = (float) std::chrono::duration_cast<std::chrono::milliseconds>(
            now - rewindAudioFadeStart).count() / 1000.0f;

    float t = elapsedSec / fadeDurationSec;
    if (t >= 1.0f) {
        audio->setGain(1.0f);
        rewindAudioFadeActive = false;
        return;
    }

    if (t < 0.0f) {
        t = 0.0f;
    }
    audio->setGain(t);
}
