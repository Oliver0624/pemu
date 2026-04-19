//
// Created by Oliver on 04/05/24.
//

#include "c2dui.h"
#include "c2dui_ui_menu_line.h"

MenuLine::MenuLine(UiMain *u, c2d::FloatRect &rect, Skin::TextGroup &tg) : RectangleShape(rect) {
    pMain = u;
    textGroup = tg;
    Font *font = pMain->getSkin()->getFont();

    name = new Text("OPTION NAME", textGroup.size, font);
    name->setFillColor(textGroup.color);
    name->setOutlineThickness(textGroup.outlineSize);
    name->setOutlineColor(textGroup.outlineColor);
    name->setOrigin(Origin::Left);
    name->setPosition(2 * pMain->getScaling().x, MenuLine::getSize().y / 2);
    name->setSizeMax((MenuLine::getSize().x * 0.55f), 0);
    MenuLine::add(name);

    value = new Text("OPTION VALUE", textGroup.size, font);
    value->setFillColor(textGroup.color);
    value->setOutlineThickness(textGroup.outlineSize);
    value->setOutlineColor(textGroup.outlineColor);
    value->setOrigin(Origin::Left);
    value->setPosition((MenuLine::getSize().x * 0.6f), MenuLine::getSize().y / 2);
    value->setSizeMax(MenuLine::getSize().x * 0.38f, 0);
    MenuLine::add(value);
    valueBasePos = value->getPosition();

    valueIncoming = new Text("OPTION VALUE", textGroup.size, font);
    valueIncoming->setFillColor(textGroup.color);
    valueIncoming->setOutlineThickness(textGroup.outlineSize);
    valueIncoming->setOutlineColor(textGroup.outlineColor);
    valueIncoming->setOrigin(Origin::Left);
    valueIncoming->setPosition(valueBasePos);
    valueIncoming->setSizeMax(MenuLine::getSize().x * 0.38f, 0);
    valueIncoming->setVisibility(Visibility::Hidden);
    MenuLine::add(valueIncoming);

    sprite = new Sprite();
    MenuLine::add(sprite);

    valueTweenCurrent = new TweenPosition(valueBasePos, valueBasePos, 0.1f);
    valueTweenCurrent->setState(TweenState::Stopped);
    value->add(valueTweenCurrent);

    valueTweenIncoming = new TweenPosition(valueBasePos, valueBasePos, 0.1f);
    valueTweenIncoming->setState(TweenState::Stopped);
    valueIncoming->add(valueTweenIncoming);
}

void MenuLine::set(const Option &opt) {
    option = opt;
    resetValueAnimation();

    setVisibility(Visibility::Visible);
    sprite->setVisibility(Visibility::Hidden);

    // reset
    name->setString(option.getDisplayName());
    value->setVisibility(Visibility::Visible);
    setFillColor(Color::Transparent);

    if (option.getFlags() & Option::Flags::INPUT) {
        Skin::Button *button = pMain->getSkin()->getButton(option.getValueInt());
        if (button && option.getId() < Option::Id::JOY_DEADZONE) {
            if (button->texture) {
                sprite->setTexture(button->texture, true);
                sprite->setVisibility(Visibility::Visible);
                value->setVisibility(Visibility::Hidden);
                float scaling = std::min(
                        getSize().x / (float) sprite->getSize().x,
                        getSize().y / (float) sprite->getSize().y);
                sprite->setScale(scaling, scaling);
                sprite->setPosition((MenuLine::getSize().x * 0.6f), MenuLine::getSize().y / 2);
                sprite->setOrigin(Origin::Left);
            } else {
                sprite->setVisibility(Visibility::Hidden);
                value->setVisibility(Visibility::Visible);
                value->setString(button->name);
            }
        } else {
            char btn[16];
            snprintf(btn, 16, "%i", option.getValueInt());
            value->setVisibility(Visibility::Visible);
            value->setString(btn);
        }
    } else if (option.getFlags() & Option::Flags::MENU) {
        value->setVisibility(Visibility::Hidden);
        setFillColor(pMain->getUiMenu()->getOutlineColor());
    } else {
        value->setVisibility(Visibility::Visible);
        value->setString(option.getValueDisplayString());
    }
}

void MenuLine::setWithValueSlide(const Option &opt, int direction, int durationMs) {
    Option animOpt = opt;

    if (direction == 0 || durationMs <= 0) {
        set(opt);
        return;
    }

    // Do not animate non-text value representations.
    if ((animOpt.getFlags() & Option::Flags::INPUT) || (animOpt.getFlags() & Option::Flags::MENU)) {
        set(opt);
        return;
    }

    resetValueAnimation();

    option = opt;
    setVisibility(Visibility::Visible);
    sprite->setVisibility(Visibility::Hidden);
    setFillColor(Color::Transparent);
    name->setString(option.getDisplayName());
    value->setVisibility(Visibility::Visible);
    value->setOutlineColor(textGroup.outlineColor);
    valueIncoming->setVisibility(Visibility::Visible);
    valueIncoming->setOutlineColor(textGroup.outlineColor);

    std::string oldValue = value->getString();
    std::string newValue = option.getValueDisplayString();
    if (oldValue.empty() || oldValue == newValue) {
        value->setString(newValue);
        valueIncoming->setVisibility(Visibility::Hidden);
        return;
    }

    valueIncoming->setString(newValue);

    float offset = getSize().x * 0.08f;
    if (offset < 8.0f) {
        offset = 8.0f;
    }

    float incomingStartX = direction < 0 ? valueBasePos.x - offset : valueBasePos.x + offset;
    float outgoingEndX = direction < 0 ? valueBasePos.x + offset : valueBasePos.x - offset;

    value->setPosition(valueBasePos);
    valueIncoming->setPosition(incomingStartX, valueBasePos.y);

    float sec = (float) durationMs * 0.001f;
    valueTweenCurrent->setFromTo(valueBasePos, {outgoingEndX, valueBasePos.y}, sec);
    valueTweenIncoming->setFromTo({incomingStartX, valueBasePos.y}, valueBasePos, sec);
    valueTweenCurrent->play(TweenDirection::Forward, true);
    valueTweenIncoming->play(TweenDirection::Forward, true);
}

void MenuLine::onUpdate() {
    RectangleShape::onUpdate();

    if (!valueIncoming->isVisible()) {
        return;
    }

    if (valueTweenCurrent->getState() == TweenState::Stopped
        && valueTweenIncoming->getState() == TweenState::Stopped) {
        value->setString(valueIncoming->getString());
        value->setPosition(valueBasePos);
        valueIncoming->setVisibility(Visibility::Hidden);
        valueIncoming->setPosition(valueBasePos);
    }
}

void MenuLine::resetValueAnimation() {
    if (valueTweenCurrent) {
        valueTweenCurrent->setState(TweenState::Stopped);
    }
    if (valueTweenIncoming) {
        valueTweenIncoming->setState(TweenState::Stopped);
    }

    if (valueIncoming && valueIncoming->isVisible()) {
        value->setString(valueIncoming->getString());
    }

    if (value) {
        value->setPosition(valueBasePos);
    }
    if (valueIncoming) {
        valueIncoming->setVisibility(Visibility::Hidden);
        valueIncoming->setPosition(valueBasePos);
    }
}
