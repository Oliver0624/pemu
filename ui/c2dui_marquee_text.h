//
// Created by cpasjuste on 03/04/26.
//

#ifndef PEMU_C2DUI_MARQUEE_TEXT_H
#define PEMU_C2DUI_MARQUEE_TEXT_H

#include "cross2d/c2d.h"

namespace c2dui {

    class MarqueeText : public c2d::Text {
    public:

        explicit MarqueeText(const std::string &string,
                             unsigned int characterSize = C2D_DEFAULT_CHAR_SIZE,
                             c2d::Font *font = nullptr);

        void setString(const std::string &string) override;

        void setOutlineThickness(float thickness) override;

        void setPosition(float x, float y) override;

        void setPosition(const c2d::Vector2f &position) override;

        void setSizeMax(float width, float height);

        void setSizeMax(const c2d::Vector2f &size);

    protected:

        void onUpdate() override;

        void onDraw(c2d::Transform &transform, bool draw = true) override;

    private:

        void refreshState();

        void applyLayout();

        float getVisibleWidth() const;

        float getRenderScrollOffset() const;

        std::string m_fullString;
        c2d::Text m_measureText;
        c2d::Vector2f m_anchorPosition = {0, 0};
        float m_maxWidth = 0;
        float m_maxHeight = 0;
        float m_scrollOffset = 0;
        float m_maxScrollOffset = 0;
        float m_pauseRemaining = 0;
        int m_direction = 1;
        bool m_overflowing = false;
    };
}

#endif //PEMU_C2DUI_MARQUEE_TEXT_H
