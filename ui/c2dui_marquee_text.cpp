//
// Created by cpasjuste on 03/04/26.
//

#include "c2dui_marquee_text.h"
#include <algorithm>
#include <cmath>

using namespace c2d;
using namespace c2dui;

namespace {

    constexpr float MARQUEE_SPEED_PIXELS_PER_SECOND = 36.0f;
    constexpr float MARQUEE_END_PAUSE_SECONDS = 1.0f;
    constexpr float MARQUEE_RIGHT_PADDING_PIXELS = 2.0f;
}

MarqueeText::MarqueeText(const std::string &string, unsigned int characterSize, Font *font)
        : Text(string, characterSize, font), m_fullString(string), m_measureText(string, characterSize, font) {
    m_measureText.setSizeMax(0, 0);
}

void MarqueeText::setString(const std::string &string) {
    m_fullString = string;
    refreshState();
}

void MarqueeText::setOutlineThickness(float thickness) {
    Text::setOutlineThickness(thickness);
    m_measureText.setOutlineThickness(thickness);
    refreshState();
}

void MarqueeText::setPosition(float x, float y) {
    m_anchorPosition = {x, y};
    applyLayout();
}

void MarqueeText::setPosition(const Vector2f &position) {
    setPosition(position.x, position.y);
}

void MarqueeText::setSizeMax(float width, float height) {
    m_maxWidth = width;
    m_maxHeight = height;
    refreshState();
}

void MarqueeText::setSizeMax(const Vector2f &size) {
    setSizeMax(size.x, size.y);
}

void MarqueeText::onUpdate() {
    Text::onUpdate();

    if (!m_overflowing || m_fullString.empty()) {
        return;
    }

    float dt = c2d_renderer != nullptr ? c2d_renderer->getDeltaTime().asSeconds() : 0;
    if (dt <= 0) {
        return;
    }

    if (m_pauseRemaining > 0) {
        m_pauseRemaining = std::max(0.0f, m_pauseRemaining - dt);
        return;
    }

    float nextOffset = m_scrollOffset + (MARQUEE_SPEED_PIXELS_PER_SECOND * dt * (float) m_direction);
    if (m_direction > 0 && nextOffset >= m_maxScrollOffset) {
        m_scrollOffset = m_maxScrollOffset;
        m_direction = -1;
        m_pauseRemaining = MARQUEE_END_PAUSE_SECONDS;
    } else if (m_direction < 0 && nextOffset <= 0) {
        m_scrollOffset = 0;
        m_direction = 1;
        m_pauseRemaining = MARQUEE_END_PAUSE_SECONDS;
    } else {
        m_scrollOffset = nextOffset;
    }

    applyLayout();
}

void MarqueeText::onDraw(Transform &transform, bool draw) {
    if (!getFont() || m_fullString.empty()) {
        return;
    }

    if (draw && c2d_renderer != nullptr && m_overflowing) {
        FloatRect clipRect = {
                getRenderScrollOffset(),
                getLocalBounds().top,
                getVisibleWidth(),
                std::max(getLocalBounds().height, (float) getCharacterSize())
        };
        Transform combined = transform * getTransform();
        c2d_renderer->pushClipRect(combined.transformRect(clipRect));
        Text::onDraw(transform, draw);
        c2d_renderer->popClipRect();
        return;
    }

    Text::onDraw(transform, draw);
}

void MarqueeText::refreshState() {
    m_measureText.setFont(getFont());
    m_measureText.setCharacterSize(getCharacterSize());
    m_measureText.setStyle(getStyle());
    m_measureText.setOutlineThickness(getOutlineThickness());
    m_measureText.setString(m_fullString);
    m_measureText.setSizeMax(0, 0);

    Text::setString(m_fullString);
    Text::setSizeMax(0, m_maxHeight);

    m_scrollOffset = 0;
    m_maxScrollOffset = 0;
    m_direction = 1;
    m_pauseRemaining = MARQUEE_END_PAUSE_SECONDS;
    m_overflowing = getFont() != nullptr && !m_fullString.empty() &&
                    m_measureText.getLocalBounds().width > getVisibleWidth();
    if (m_overflowing) {
        m_maxScrollOffset = std::max(0.0f, m_measureText.getLocalBounds().width - getVisibleWidth());
    }
    applyLayout();
}

void MarqueeText::applyLayout() {
    Text::setString(m_fullString);
    Text::setPosition(m_anchorPosition.x - getRenderScrollOffset(), m_anchorPosition.y);
}

float MarqueeText::getVisibleWidth() const {
    return std::max(0.0f, std::floor(m_maxWidth - std::ceil(getOutlineThickness()) - MARQUEE_RIGHT_PADDING_PIXELS));
}

float MarqueeText::getRenderScrollOffset() const {
    return std::clamp(std::floor(m_scrollOffset), 0.0f, m_maxScrollOffset);
}
