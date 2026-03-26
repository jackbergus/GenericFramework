// NetworkEnum.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - Giacomo Bergami
//
// GeneralFramework is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  GeneralFramework is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GeneralFramework. If not, see <http://www.gnu.org/licenses/>.

#ifndef FLOATMAX_NETWORKENUM_H
#define FLOATMAX_NETWORKENUM_H
#include <type_traits>
#include <vector>
#include <functional>
#include <iomanip>

#include "../../../submodules/narcissus/submodule/magic_enum/include/magic_enum/magic_enum.hpp"
#include "jackbergus/data/packed.h"

PACK(struct _enum {});

template <typename enum_type,
         enum_type default_value>
struct Enum : public _enum {
    using T = typename std::underlying_type<enum_type>::type;
    static constexpr bool is_ufloat = false;
    static constexpr bool to_signed = std::is_signed_v<T>;
    static_assert(std::is_integral_v<T>, "The underlying type of the enum shall be an integral value");
    static_assert(std::is_enum<enum_type>::value, "Enum type required");

    Enum(enum_type val = default_value) {
        value = val;
    }

    Enum& operator=(T val) {
        value = static_cast<enum_type>(val);
        for (const auto& ob : observers)
            ob();
        return *this;
    }

    void update() {
        for (auto& ref : observers)
            ref();
    }

    Enum& operator=(enum_type val) {
        value = val;
        for (const auto& ob : observers)
            ob();
        return *this;
    }

    operator T() const {
        return static_cast<T>(value);
    }

    operator bool() const {
        return true;
    }

    bool operator==(Enum& val) {
        return value == val.value;
    }

    void addObserver(std::function<void()> f) {
        observers.emplace_back(f);
    }
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << std::quoted(magic_enum::enum_name(value));
        return ss.str();
    }

private:
    std::vector<std::function<void()>> observers;
    enum_type value;
};

#endif //FLOATMAX_NETWORKENUM_H