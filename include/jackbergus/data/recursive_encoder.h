// recursive_encoder.h
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

#ifndef FLOATMAX_RECURSIVE_ENCODER_H
#define FLOATMAX_RECURSIVE_ENCODER_H

#include "jackbergus/data/NetworkEnum.h"
#include "jackbergus/data/NetworkFloat.h"
#include "jackbergus/data/NetworkInt.h"
#include "refl.hpp"
#include "jackbergus/data/template_typing.h"
#include <functional>

template<typename T, uint64_t idx>
using get_field_type = typename refl::trait::get_t<idx, refl::member_list<T> >::value_type;

template<typename T, uint64_t idx>
using get_field_t = typename refl::trait::get_t<idx, refl::member_list<T> >;

template<typename T, uint64_t idx>
using get_field2 = typename refl::descriptor::member_descriptor_base<T, idx>;

template<typename T, typename V>
bool encode(T &dst, const V &src);

template<typename T, typename V>
bool encode_extended(T* dst, V &src);

template<typename V>
bool update(V &src);

template<typename T, typename V>
bool decode(T &dst, const V &src);

template<typename T, typename U, int x, int to>
struct static_for {
    bool decode(const T &src, U &dst) {
        auto val = static_for<T, U, x + 1, to>{}.decode(src, dst);
        using TIth = get_field_type<T, x>;
        using UIth = get_field_type<U, x>;
        if constexpr (is_std_array<UIth>::value && std::is_array<TIth>::value) {
            for (uint64_t i = 0, N = std_array_size<UIth>::size; i<N; i++) {
                if (!::decode(get_field_t<U, x>::get(dst)[i], get_field_t<T, x>::get(src)[i]))
                    val = false;
            }
        } else if constexpr (std::is_base_of<_float, UIth>::value && std::is_integral<TIth>::value) {
            auto val_ = get_field_t<T, x>::get(src);
            get_field_t<U, x>::get(dst) = (typename UIth::T) val_;
        } else if constexpr (std::is_base_of<_int, UIth>::value ) {
            if constexpr (std::is_same<TIth, typename UIth::T>::value) {
                auto val_ = get_field_t<T, x>::get(src);
                get_field_t<U, x>::get(dst).decode(val_);
            } else {
                val = false;
            }
        } else if constexpr (std::is_base_of<_enum, UIth>::value ) {
            if constexpr (std::is_same<TIth, typename UIth::T>::value) {
                auto val_ = get_field_t<T, x>::get(src);
                get_field_t<U, x>::get(dst) = val_;
            } else {
                val = false;
            }
        } else if constexpr (!std::is_fundamental_v<TIth> && !std::is_fundamental_v<UIth>) {
            val = ::decode(get_field_t<U, x>::get(dst), get_field_t<T, x>::get(src));
        } else {
            val = false;
        }
        return val;
    }

    bool update(U &src) {
        auto val = static_for<T, U, x + 1, to>{}.update(src);
        using TIth = get_field_type<T, x>;
        // using UIth = get_field_type<U, x>;
        // auto ifFloat = std::is_base_of<_float, UIth>::value;
        // auto ifInt = std::is_base_of<_int, UIth>::value;

        if constexpr (is_std_array<TIth>::value) {
            for (uint64_t i = 0, N = std_array_size<TIth>::size; i<N; i++) {
                 (::update(get_field_t<U, x>::get(src)[i]));
            }
        } else if constexpr (std::is_base_of<_float, TIth>::value) {
            get_field_t<U, x>::get(src).update();
        } else if constexpr (std::is_base_of<_enum, TIth>::value) {
            get_field_t<U, x>::get(src).update();
        } else if constexpr (std::is_base_of<_int, TIth>::value) {
            get_field_t<U, x>::get(src).update();
        } else if constexpr (!std::is_fundamental<TIth>::value) {
            ::update(get_field_t<U, x>::get(src));
            // val = encode(get_field_t<T, x>::get(dst), get_field_t<U, x>::get(src));
        } else {
            val = false;
        }
        return val;
    }

    bool encode_extended(T* dst, U &src) {
        auto val = static_for<T, U, x + 1, to>{}.encode_extended(dst, src);
        using TIth = get_field_type<T, x>;
        using UIth = get_field_type<U, x>;
        auto ifFloat = std::is_base_of<_float, UIth>::value;
        auto ifInt = std::is_base_of<_int, UIth>::value;

        if constexpr (is_std_array<UIth>::value && std::is_array<TIth>::value) {
            for (uint64_t i = 0, N = std_array_size<UIth>::size; i<N; i++) {
                 (::encode_extended(&get_field_t<T, x>::get(*dst)[i], get_field_t<U, x>::get(src)[i]));
            }
        } else if constexpr (std::is_base_of<_float, UIth>::value && std::is_integral<TIth>::value) {
            auto f = [&src,dst]() {
                auto val_ = get_field_t<U, x>::get(src).encode();
                get_field_t<T, x>::get(*dst) = val_;
            };
            get_field_t<U, x>::get(src).addObserver((f));
        } else if constexpr (std::is_base_of<_enum, UIth>::value) {
            if constexpr (std::is_same<TIth, typename UIth::T>::value) {
                auto f = [&src,dst]() {
                    get_field_t<T, x>::get(*dst) = (typename UIth::T)(get_field_t<U, x>::get(src));
                };
                get_field_t<U, x>::get(src).addObserver((f));
            } else {
                val = false;
            }
        } else if constexpr (std::is_base_of<_int, UIth>::value) {
            if constexpr (std::is_same<TIth, typename UIth::T>::value) {
                auto f = [&src,dst]() {
                    auto val_ = get_field_t<U, x>::get(src).encode();
                    get_field_t<T, x>::get(*dst) = val_;
                };
                get_field_t<U, x>::get(src).addObserver((f));
            } else {
                val = false;
            }
        } else if constexpr (!std::is_fundamental<TIth>::value && !std::is_fundamental<UIth>::value) {
            ::encode_extended(&get_field_t<T, x>::get(*dst), get_field_t<U, x>::get(src));
            // val = encode(get_field_t<T, x>::get(dst), get_field_t<U, x>::get(src));
        } else {
            val = false;
        }
        return val;
    }

    bool encode(T &dst, const U &src) {
        auto val = static_for<T, U, x + 1, to>{}.encode(dst, src);
        using TIth = get_field_type<T, x>;
        using UIth = get_field_type<U, x>;
        auto ifFloat = std::is_base_of<_float, UIth>::value;
        auto ifInt = std::is_base_of<_int, UIth>::value;

        if constexpr (is_std_array<UIth>::value && std::is_array<TIth>::value) {
            for (uint64_t i = 0, N = std_array_size<UIth>::size; i<N; i++) {
                if (!::encode(get_field_t<T, x>::get(dst)[i], get_field_t<U, x>::get(src)[i]))
                    val = false;
            }
        } else if constexpr (std::is_base_of<_float, UIth>::value && std::is_integral<TIth>::value) {
            auto val_ = get_field_t<U, x>::get(src).encode();
            get_field_t<T, x>::get(dst) = val_;
        } else if constexpr (std::is_base_of<_enum, UIth>::value) {
            if constexpr (std::is_same<TIth, typename UIth::T>::value) {
                get_field_t<T, x>::get(dst) = (typename UIth::T)(get_field_t<U, x>::get(src));
            } else {
                val = false;
            }
        } else if constexpr (std::is_base_of<_int, UIth>::value) {
            if constexpr (std::is_same<TIth, typename UIth::T>::value) {
                auto val_ = get_field_t<U, x>::get(src).encode();
                get_field_t<T, x>::get(dst) = val_;
            } else {
                val = false;
            }
        } else if constexpr (!std::is_fundamental<TIth>::value && !std::is_fundamental<UIth>::value) {
            val = ::encode(get_field_t<T, x>::get(dst), get_field_t<U, x>::get(src));
            // val = encode(get_field_t<T, x>::get(dst), get_field_t<U, x>::get(src));
        } else {
            val = false;
        }
        return val;
    }
};

template<typename T, typename U, int to>
struct static_for<T, U, to, to> {

    bool encode_extended(T* dst, const U &src) {
        return true;
    }

    bool update(const U &src) {
        return true;
    }

    bool encode(T &dst, const U &src) {
        return true;
    }

    bool decode(const T &src, U &dst) {
        return true;
    }
};

template<typename T, typename V>
bool encode(T &dst, const V &src) {
    if constexpr (refl::member_list<T>::size != refl::member_list<V>::size) {
        return false;
    } else {
        return static_for<T, V, 0, refl::member_list<V>::size>{}.encode(dst, src);
    }
}

template<typename V>
bool update(V &src) {
    return static_for<V, V, 0, refl::member_list<V>::size>{}.update(src);
}

template<typename T, typename V>
bool encode_extended(T* dst, V &src) {
    if constexpr (refl::member_list<T>::size != refl::member_list<V>::size) {
        return false;
    } else {
        return static_for<T, V, 0, refl::member_list<V>::size>{}.encode_extended(dst, src);
    }
}

template<typename T, typename V>
bool decode(T &dst, const V &src) {
    if constexpr (refl::member_list<T>::size != refl::member_list<V>::size) {
        return false;
    } else {
        return static_for<V, T, 0, refl::member_list<V>::size>{}.decode(src, dst);
    }
}

#endif //FLOATMAX_RECURSIVE_ENCODER_H