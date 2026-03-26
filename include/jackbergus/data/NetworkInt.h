// NetworkInt.h
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

#ifndef INTMAX_NETWORKINT_H
#define INTMAX_NETWORKINT_H

#include "jackbergus/data/packed.h"
#include "jackbergus/data/FltForTemplateParam.h"
#include <type_traits>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <ostream>

PACK(struct _int {});

template <typename int_type = int32_t,
          int_type min = 0,
          int_type max = 10000,
          int_type LBS = 1,
          int_type ADD = 0> struct Int : public _int {
    using T = int_type;
    static constexpr bool is_ufloat = false;
    static constexpr int_type MIN = min;
    static constexpr int_type MAX = max;
    static constexpr int_type mult = LBS;
    static constexpr int_type kconst = ADD;
    static constexpr bool to_signed = std::is_signed_v<int_type>;
    static_assert(std::is_integral_v<int_type> && sizeof(int_type)<sizeof(int64_t));
    static constexpr auto min_repr = to_signed ? std::numeric_limits<int_type>::lowest() : (int_type)0;
    static constexpr auto max_repr = std::numeric_limits<int_type>::max();
    // Checking whether the resulting value will be valid after the conversion to (u)int32
    static_assert((((MIN+kconst)*mult) >= min_repr) && (((MAX+kconst)*mult<=max_repr)));

    // /**
    //  * This represents the network level encode, representing a float from an integer type
    //  */
    // Float(i32_type network_repr);
    /**
     * This provides the logical level representation of the field 
     */
    Int(int_type val = MIN);

    /**
     * This allows the definition of the logical level of the message
     */
    Int& operator=(int_type val);

    /**
     * This allows for the message decode by converting the network representation into the desired element
     */
    // Float& operator=(i32_type network_repr);

    /**
     * Encoding the value into the desired network representation
     */
    constexpr int_type encode() const;

    void decode(int_type val) {
        value = ((val)/mult)-kconst;
    }

    /**
     * Returns true if the stored value is valid
     */
    operator bool() const;

    bool operator==(Int& x) const {
        return value == x.value;
    }

    void addObserver(std::function<void()> f) {
        observers.emplace_back(f);
    }

    void update() {
        for (auto& ref : observers)
            ref();
    }

    [[nodiscard]] std::string to_string() const {
        return std::to_string(value);
    }

    /**
     * If the value is valid, then returns the logical value representation. Otherwise, it returns the first non-admissible value before the minimum allowed value
     */
    // operator float() const;
private:
    std::vector<std::function<void()>> observers;
    int_type value;
};

// template <typename i32_type,
//           typename min,
//           typename max,
//           typename LBS,
//           typename ADD>  Int<i32_type, min, max,LBS,ADD>::Float(i32_type network_repr)
// : value(((i32_type)network_repr)/mult-kconst) {
//     auto valid = ((value > MIN) || (std::abs(value - MIN)<= EPS)) &&
//     ((value < MAX) || (std::abs(value - MAX)<= EPS));
//     if (!valid) {
//         value = std::nextafterf(MIN-EPS, -INFINITY);
//     }
// }

template <typename i32_type,
          i32_type min,
          i32_type max,
          i32_type LBS,
i32_type ADD>  Int<i32_type, min, max,LBS,ADD>::Int(i32_type val) : value(val) {
    auto valid = ((val >= MIN)) && ((val <= MAX));
    if (!valid) {
        value = ((MIN-1) > MIN) ? MIN : (MIN-1);
    }
}

template <typename i32_type,
          i32_type min,
          i32_type max,
          i32_type LBS,
i32_type ADD>
Int<i32_type, min, max,LBS,ADD>& Int<i32_type, min, max,LBS,ADD>::operator=(i32_type val) {
    value = val;
    auto valid = ((val >= MIN) ) && ((val <= MAX));
    if (!valid) {
        value = ((MIN-1) > MIN) ? MIN : (MIN-1);
    }
    for (auto& ob : observers)
        ob();
    return *this;
}


// template <typename i32_type,
//           typename min,
//           typename max,
//           typename LBS,
// typename ADD>
// Int<i32_type, min, max,eps,LBS,ADD>& Int<i32_type, min, max,LBS,ADD>::operator=(i32_type network_repr) {
//     value = (((float)network_repr)/mult-kconst);
//     auto valid = ((value > MIN) || (std::abs(value - MIN)<= EPS)) &&
//     ((value < MAX) || (std::abs(value - MAX)<= EPS));
//     if (!valid) {
//         value = std::nextafterf(MIN-EPS, -INFINITY);
//     }
//     return *this;
// }


template <typename i32_type,
          i32_type min,
          i32_type max,
          i32_type LBS,
          i32_type ADD>
constexpr i32_type Int<i32_type, min, max,LBS,ADD>::encode() const {
    return (value+kconst)*mult;
}

template <typename i32_type,
          i32_type min,
          i32_type max,
          i32_type LBS,
          i32_type ADD>
Int<i32_type, min, max,LBS,ADD>::operator bool() const {
    return (((value >= MIN)) && ((value <= MAX)));
}

// template <typename i32_type,
//           typename min,
//           typename max,
//           typename LBS,
//           typename ADD>
// Int<i32_type, min, max,LBS,ADD>::operator float() const {
//     return value;
// }

#endif //FLOATMAX_NETWORKFLOAT_H