//
// Created by Oliver on 04/25/2024
//

#ifndef PGBA_UIEMU_H
#define PGBA_UIEMU_H

#include <string>
#include <vector>

struct mCore;

class PGBAUiEmu : public c2dui::UiEmu {
public:
    explicit PGBAUiEmu(c2dui::UiMain *ui);

    int  load(const ss_api::Game &game) override;
    void stop() override;
    bool onInput(c2d::Input::Player *players) override;
    void onUpdate() override;

    bool serializeState(std::vector<uint8_t> &out) override;
    bool deserializeState(const std::vector<uint8_t> &data) override;
    void renderPreviewFrame() override;
    void clearAudio() override;
    bool supportsRewind() override { return true; }
};

#endif //PGBA_UIEMU_H
