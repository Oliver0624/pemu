//
// Created by cpasjuste on 03/04/26.
//

#ifndef PEMU_C2DUI_ANIMATED_PAGE_TEXT_H
#define PEMU_C2DUI_ANIMATED_PAGE_TEXT_H

#include <array>
#include "cross2d/c2d.h"

namespace c2dui {
    class UiMain;
}

#include "c2dui_skin.h"

namespace c2dui {

    class AnimatedPageText : public c2d::RectangleShape {
    public:

        AnimatedPageText();

        ~AnimatedPageText() override;

        void applyTextGroup(const Skin::TextGroup &group, c2d::Font *font);

        void setString(const std::string &string);

        void scrollText(c2d::Text::ScrollType type);

    protected:

        void onUpdate() override;

        void onDraw(c2d::Transform &transform, bool draw = true) override;

    private:

        void applyTextStyle(c2d::Text *text);

        void resetAnimation();

        void syncPage(c2d::Text *text, size_t page);

        void setChildOffset(c2d::Text *text, float y);

        c2d::Text *m_currentText = nullptr;
        c2d::Text *m_nextText = nullptr;
        c2d::Font *m_font = nullptr;
        std::string m_fullString;
        c2d::Origin m_textOrigin = c2d::Origin::TopLeft;
        unsigned int m_characterSize = C2D_DEFAULT_CHAR_SIZE;
        c2d::Color m_fillColor = c2d::Color::White;
        c2d::Color m_outlineColor = c2d::Color::Black;
        uint32_t m_overflow = c2d::Text::Overflow::Clamp;
        float m_outlineThickness = 0;
        float m_animationElapsed = 0;
        float m_animationDuration = 0.14f;
        float m_slideDistance = 0;
        int m_animationDirection = 0;
        size_t m_currentPage = 0;
        size_t m_targetPage = 0;
        bool m_animating = false;
    };
}

#endif //PEMU_C2DUI_ANIMATED_PAGE_TEXT_H
