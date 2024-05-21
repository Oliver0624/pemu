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

    sprite = new Sprite();
    MenuLine::add(sprite);
}

void MenuLine::set(const Option &opt) {
    option = opt;

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
