//
// Created by cpasjuste on 05/12/16.
//

#include <c2dui.h>
#include "c2dui_option.h"


Option::Option(const std::string &text,
               const std::vector<std::pair<std::string, std::string>> &options,
               int defaultValueIndex,
               int id,
               unsigned int flags,
               const std::string &displayName) {
    this->flags = flags;
    this->name = text;
    this->options = options;
    this->id = id;
    this->cur_index = defaultValueIndex;
    adjustCurIndex();
    if (displayName.length() == 0) {
        this->display_name = this->name;
    } else {
        this->display_name = displayName;
    }
}

Option::Option(const Option &option)
{
    name = option.name;
    info = option.info;
    options = option.options;
    cur_index = option.cur_index;
    adjustCurIndex();
    flags = option.flags;
    id = option.id;
    display_name = option.display_name;
}

std::string Option::getName() const {
    return name;
}

std::string Option::getDisplayName() const {
    return display_name;
}

std::string Option::getInfo() const {
    return info;
}

void Option::setInfo(const std::string &inf) {
    info = inf;
}

std::vector<std::string> Option::getValues() {
    std::vector<std::string> values;
    for (const auto &v : options) {
        values.push_back(v.first);
    }
    return values;
}

std::string Option::getValueString() const {
    return (0 <= cur_index && cur_index < (int)options.size()) ? options.at(cur_index).first : "";
}

std::string Option::getValueDisplayString() const {
    return (0 <= cur_index && cur_index < (int)options.size()) ? options.at(cur_index).second : "";
}

void Option::setValueString(const std::string &value) {
    int size = (int) options.size();
    if (0 == size) {
        return;
    }

    bool found = false;
    for (int i = 0; i < size; i++) {
        if (options.at(i).first == value) {
            cur_index = i;
            found = true;
            break;
        }
    }

    if (!found) {
        cur_index = 0;
    }
}

int Option::getValueInt(int defValue) {
    return (0 <= cur_index && cur_index < (int)options.size()) ? c2d::Utility::parseInt(options.at(cur_index).first, defValue) : defValue;
}

void Option::setValueInt(int value) {
    setValueString(std::to_string(value));
}

bool Option::getValueBool() {
    return getIndex() > 0;
}

void Option::setValueBool(bool value) {
    setIndex(value ? 1 : 0);
}

int Option::getId() {
    return id;
}

void Option::setId(int _id) {
    id = _id;
}

unsigned int Option::getFlags() {
    return flags;
}

void Option::setFlags(unsigned int _flags) {
    flags = _flags;
}

void Option::next() {
    if (flags & Flags::INPUT) {
        return;
    }

    if (options.size() <= 1) {
        return;
    }

    if (flags & Flags::INTEGER) {
        int value = getValueInt();
        setValueInt(value + 1);
        return;
    }

    cur_index++;
    if (cur_index < 0 || cur_index >= (int)options.size()) {
        cur_index = 0;
    }
}

void Option::prev() {
    if (flags & Flags::INPUT) {
        return;
    }

    if (options.size() <= 1) {
        return;
    }

    if (flags & Flags::INTEGER) {
        int value = getValueInt();
        setValueInt(value - 1);
        return;
    }

    cur_index--;
    if (cur_index < 0 || cur_index >= (int)options.size()) {
        cur_index = options.size() - 1;
    }
}

int Option::getIndex() {
    return cur_index;
}

void Option::setIndex(int index) {
    cur_index = index;
    adjustCurIndex();
}

void Option::set(const Option &option) {
    name = option.name;
    info = option.info;
    options = option.options;
    cur_index = option.cur_index;
    adjustCurIndex();
    flags = option.flags;
    id = option.id;
    if (option.display_name != name) {
        display_name = option.display_name;
    }
}

int Option::size() {
    return (int) options.size();
}

void Option::adjustCurIndex() {
    if (0 == options.size()) {
        cur_index = -1;
    } else if (cur_index < 0) {
        cur_index = 0;
    } else if (cur_index >= (int)options.size()) {
        cur_index = options.size() - 1;
    }
}