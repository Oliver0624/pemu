//
// Created by cpasjuste on 05/12/16.
//

#include <c2dui.h>
#include "c2dui_option.h"


Option::Option(const std::string &text,
               const std::vector<std::string> &options,
               int defaultValueIndex,
               int id,
               unsigned int flags,
               const std::string &displayName) {
    this->flags = flags;
    this->name = text;
    this->options = options;
    this->id = id;
    if (defaultValueIndex < (int) this->options.size()) {
        current_option = this->options.at(defaultValueIndex);
    }
    if (displayName.length() == 0) {
        this->display_name = this->name;
    } else {
        this->display_name = displayName;
    }
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

std::vector<std::string> *Option::getValues() {
    return &options;
}

std::string Option::getValueString() const {
    return current_option;
}

void Option::setValueString(const std::string &value) {
    current_option = value;
}

int Option::getValueInt(int defValue) {
    return c2d::Utility::parseInt(current_option, defValue);
}

void Option::setValueInt(int value) {
    current_option = std::to_string(value);
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
    bool found = false;

    if (flags & Flags::INPUT) {
        return;
    }

    if (flags & Flags::INTEGER) {
        int value = getValueInt();
        setValueInt(value + 1);
        return;
    }

    int size = (int) options.size();
    for (int i = 0; i < size; i++) {
        if (options.at(i) == current_option) {
            if (i + 1 < size) {
                current_option = options.at(i + 1);
            } else {
                current_option = options.at(0);
            }
            found = true;
            break;
        }
    }

    // fallback
    if (!found) {
        current_option = options.at(0);
    }
}

void Option::prev() {
    bool found = false;

    if (flags & Flags::INPUT) {
        return;
    }

    if (flags & Flags::INTEGER) {
        int value = getValueInt();
        setValueInt(value - 1);
        return;
    }

    int size = (int) options.size();
    for (int i = size - 1; i > -1; i--) {
        if (options.at(i) == current_option) {
            if (i - 1 > -1) {
                current_option = options.at(i - 1);
            } else {
                current_option = options.at(size - 1);
            }
            found = true;
            break;
        }
    }

    // fallback
    if (!found) {
        current_option = options.at(0);
    }
}

int Option::getIndex() {
    for (size_t i = 0; i < options.size(); i++) {
        if (current_option == options.at(i)) {
            return (int) i;
        }
    }
    return 0;
}

void Option::setIndex(int index) {
    if ((size_t) index < options.size()) {
        current_option = options.at(index);
    }
}

void Option::set(const Option &option) {
    name = option.name;
    info = option.info;
    options = option.options;
    current_option = option.current_option;
    flags = option.flags;
    id = option.id;
    display_name = option.display_name;
}

int Option::size() {
    return (int) options.size();
}
