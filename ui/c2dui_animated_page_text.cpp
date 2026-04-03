//
// Created by cpasjuste on 03/04/26.
//

#include "c2dui_animated_page_text.h"
#include <algorithm>

using namespace c2d;
using namespace c2dui;

namespace {

    float easeOutCubic(float t) {
        float inv = 1.0f - t;
        return 1.0f - (inv * inv * inv);
    }
}

AnimatedPageText::AnimatedPageText() : RectangleShape({16, 16}) {
    setFillColor(Color::Transparent);
    m_currentText = new Text();
    m_nextText = new Text();
    add(m_currentText);
    add(m_nextText);
    m_nextText->setVisibility(Visibility::Hidden);
}

AnimatedPageText::~AnimatedPageText() = default;

void AnimatedPageText::applyTextGroup(const Skin::TextGroup &group, Font *font) {
    m_font = font;
    m_characterSize = group.size;
    m_textOrigin = group.origin;
    m_fillColor = group.color;
    m_outlineColor = group.outlineColor;
    m_outlineThickness = group.outlineSize;
    m_overflow = group.overflow;

    setPosition(group.rect.left, group.rect.top);
    setSize(group.rect.width, group.rect.height);
    setOrigin(Origin::TopLeft);

    applyTextStyle(m_currentText);
    applyTextStyle(m_nextText);
    resetAnimation();
}

void AnimatedPageText::setString(const std::string &string) {
    m_fullString = string;
    m_currentPage = 0;
    m_targetPage = 0;
    applyTextStyle(m_currentText);
    m_currentText->setString(m_fullString);
    applyTextStyle(m_nextText);
    m_nextText->setString(m_fullString);
    resetAnimation();
}

void AnimatedPageText::scrollText(Text::ScrollType type) {
    if (type == Text::ScrollCurrent || m_animating || m_currentText == nullptr || m_fullString.empty()) {
        return;
    }

    if (type == Text::ScrollLeft && m_currentPage == 0) {
        return;
    }

    applyTextStyle(m_nextText);
    m_nextText->setString(m_fullString);
    m_nextText->getLocalBounds();
    syncPage(m_nextText, m_currentPage);
    size_t currentPage = m_nextText->getScrollPage();
    m_nextText->scrollText(type);
    m_nextText->getLocalBounds();
    size_t nextPage = m_nextText->getScrollPage();
    if (nextPage == currentPage) {
        return;
    }

    m_targetPage = nextPage;
    m_animationDirection = (type == Text::ScrollRight) ? -1 : 1;
    m_animationElapsed = 0;
    m_animating = true;
    m_slideDistance = std::max(getSize().y, (float) m_characterSize);
    m_nextText->setVisibility(Visibility::Visible);
    setChildOffset(m_currentText, 0);
    setChildOffset(m_nextText, (float) -m_animationDirection * m_slideDistance);
}

void AnimatedPageText::onUpdate() {
    if (m_animating) {
        float dt = c2d_renderer != nullptr ? c2d_renderer->getDeltaTime().asSeconds() : 0;
        if (dt > 0) {
            m_animationElapsed = std::min(m_animationDuration, m_animationElapsed + dt);
            float progress = m_animationDuration > 0 ? m_animationElapsed / m_animationDuration : 1.0f;
            float eased = easeOutCubic(std::clamp(progress, 0.0f, 1.0f));

            setChildOffset(m_currentText, eased * (float) m_animationDirection * m_slideDistance);
            setChildOffset(m_nextText, (eased - 1.0f) * (float) m_animationDirection * m_slideDistance);

            if (m_animationElapsed >= m_animationDuration) {
                std::swap(m_currentText, m_nextText);
                m_currentPage = m_targetPage;
                resetAnimation();
            }
        }
    }

    RectangleShape::onUpdate();
}

void AnimatedPageText::onDraw(Transform &transform, bool draw) {
    if (draw && c2d_renderer != nullptr) {
        Transform combined = transform * getTransform();
        c2d_renderer->pushClipRect(combined.transformRect({0, 0, getSize().x, getSize().y}));
        RectangleShape::onDraw(transform, draw);
        c2d_renderer->popClipRect();
        return;
    }

    RectangleShape::onDraw(transform, draw);
}

void AnimatedPageText::applyTextStyle(Text *text) {
    text->setFont(m_font);
    text->setCharacterSize(m_characterSize);
    text->setFillColor(m_fillColor);
    text->setOutlineColor(m_outlineColor);
    text->setOutlineThickness(m_outlineThickness);
    text->setOrigin(m_textOrigin);
    text->setOverflow(m_overflow);
    text->setSizeMax(getSize().x, getSize().y);
}

void AnimatedPageText::resetAnimation() {
    m_animating = false;
    m_animationElapsed = 0;
    m_animationDirection = 0;
    setChildOffset(m_currentText, 0);
    setChildOffset(m_nextText, 0);
    m_nextText->setVisibility(Visibility::Hidden);
}

void AnimatedPageText::syncPage(Text *text, size_t page) {
    for (size_t i = 0; i < page; ++i) {
        text->scrollText(Text::ScrollRight);
        text->getLocalBounds();
    }
}

void AnimatedPageText::setChildOffset(Text *text, float y) {
    text->setPosition(0, y);
}
