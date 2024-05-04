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

        UiMain *pMain = nullptr;
        c2d::Text *name = nullptr;
        c2d::Text *value = nullptr;
        c2d::Sprite *sprite = nullptr;
        Skin::TextGroup textGroup;
        Option option;

    };
}

#endif //C2DUI_UI_MENU_LINE_H
