//
// Created by Oliver on 04/05/24.
//

#ifndef C2DUI_UI_MENU_LINE_H
#define C2DUI_UI_MENU_LINE_H

namespace c2dui {
    class MenuLine : public c2d::RectangleShape {

    public:

        MenuLine(UiMain *u, c2d::FloatRect &rect, Skin::TextGroup &tg);

        void set(const Option &opt);

        void setWithValueSlide(const Option &opt, int direction, int durationMs = 100);

        void onUpdate() override;

        UiMain *pMain = nullptr;
        c2d::Text *name = nullptr;
        c2d::Text *value = nullptr;
        c2d::Text *valueIncoming = nullptr;
        c2d::Sprite *sprite = nullptr;
        c2d::TweenPosition *valueTweenCurrent = nullptr;
        c2d::TweenPosition *valueTweenIncoming = nullptr;
        c2d::Vector2f valueBasePos{};
        Skin::TextGroup textGroup;
        Option option;

    private:
        void resetValueAnimation();
    };
}

#endif //C2DUI_UI_MENU_LINE_H
