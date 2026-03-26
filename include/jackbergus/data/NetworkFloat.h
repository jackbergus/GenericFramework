// NetworkFloat.h
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

#ifndef FLOATMAX_NETWORKFLOAT_H
#define FLOATMAX_NETWORKFLOAT_H

#include "jackbergus/data/packed.h"
#include "jackbergus/data/FltForTemplateParam.h"
#include <type_traits>
#include <cstdint>
#include <cmath>
#include <vector>
#include <functional>

PACK(struct _float {});

template <typename i32_type,
          typename min = FZero,
          typename max = FMax,
          typename eps = FEps,
          typename LBS = FOne,
          typename ADD = FZero> struct Float : public _float {
    using T = i32_type;
    static constexpr bool is_ufloat = true;
    static constexpr float MIN = min();
    static constexpr float MAX = max();
    static constexpr float EPS = eps();
    static constexpr float mult = LBS();
    static constexpr float kconst = ADD();
    static constexpr bool to_signed = std::is_signed_v<i32_type>;
    static_assert(std::is_integral_v<i32_type> && sizeof(i32_type)==sizeof(int32_t) && (!std::is_same_v<i32_type, float>));
    static constexpr auto min_repr = to_signed ? std::numeric_limits<i32_type>::lowest() : (i32_type)0;
    static constexpr auto max_repr = std::numeric_limits<i32_type>::max();
    // Checking whether the resulting value will be valid after the conversion to (u)int32
    static_assert((static_cast<i32_type>((MIN+kconst)*mult) >= min_repr) && (static_cast<i32_type>((MAX+kconst)*mult<=max_repr)));

    /**
     * This represents the network level encode, representing a float from an integer type
     */
    Float(i32_type network_repr);
    /**
     * This provides the logical level representation of the field 
     */
    Float(float val = MIN);

    /**
     * This allows the definition of the logical level of the message
     */
    Float& operator=(float val);

    /**
     * This allows for the message decode by converting the network representation into the desired element
     */
    Float& operator=(i32_type network_repr);

    /**
     * Encoding the value into the desired network representation
     */
    constexpr i32_type encode() const;

    bool operator==(Float& x) const {
        return std::abs(value - x.value) <= EPS;
    }

    /**
     * Returns true if the stored value is valid
     */
    operator bool() const;

    /**
     * If the value is valid, then returns the logical value representation. Otherwise, it returns the first non-admissible value before the minimum allowed value
     */
    operator float() const;

    void update() {
        for (auto& ref : observers)
            ref();
    }

    void addObserver(std::function<void()> f) {
        observers.emplace_back(f);
    }

    [[nodiscard]] std::string to_string() const {
        return std::to_string(value);
    }

private:
    float value;
    std::vector<std::function<void()>> observers;
};

template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
          typename ADD>  Float<i32_type, min, max,eps,LBS,ADD>::Float(i32_type network_repr)
: value(((float)network_repr)/mult-kconst) {
    auto valid = ((value > MIN) || (std::abs(value - MIN)<= EPS)) &&
    ((value < MAX) || (std::abs(value - MAX)<= EPS));
    if (!valid) {
        value = std::nextafterf(MIN-EPS, -INFINITY);
    }
}

template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
typename ADD>  Float<i32_type, min, max,eps,LBS,ADD>::Float(float val) : value(val) {
    auto valid = ((val > MIN) || (std::abs(val - MIN)<= EPS)) &&
            ((val < MAX) || (std::abs(val - MAX)<= EPS));
    if (!valid) {
        value = std::nextafterf(MIN-EPS, -INFINITY);
    }
}

template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
typename ADD>
Float<i32_type, min, max,eps,LBS,ADD>& Float<i32_type, min, max,eps,LBS,ADD>::operator=(float val) {
    value = val;
    auto valid = ((val > MIN) || (std::abs(val - MIN)<= EPS)) &&
    ((val < MAX) || (std::abs(val - MAX)<= EPS));
    if (!valid) {
        value = std::nextafterf(MIN-EPS, -INFINITY);
    }
    for (const auto& ob : observers)
        ob();
    return *this;
}


template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
typename ADD>
Float<i32_type, min, max,eps,LBS,ADD>& Float<i32_type, min, max,eps,LBS,ADD>::operator=(i32_type network_repr) {
    value = (((float)network_repr)/mult-kconst);
    auto valid = ((value > MIN) || (std::abs(value - MIN)<= EPS)) &&
    ((value < MAX) || (std::abs(value - MAX)<= EPS));
    if (!valid) {
        value = std::nextafterf(MIN-EPS, -INFINITY);
    }
    for (const auto& ob : observers)
        ob();
    return *this;
}


template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
          typename ADD>
constexpr i32_type Float<i32_type, min, max,eps,LBS,ADD>::encode() const {
    return (value+kconst)*mult;
}



template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
          typename ADD>
Float<i32_type, min, max,eps,LBS,ADD>::operator bool() const {
    return ((value > MIN) || (std::abs(value - MIN)<= EPS)) &&
    ((value < MAX) || (std::abs(value - MAX)<= EPS)) ;
}

template <typename i32_type,
          typename min,
          typename max,
          typename eps,
          typename LBS,
          typename ADD>
Float<i32_type, min, max,eps,LBS,ADD>::operator float() const {
    return value;
}

#endif //FLOATMAX_NETWORKFLOAT_H