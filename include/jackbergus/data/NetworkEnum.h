//
// Created by gyankos on 23/03/26.
//

#ifndef FLOATMAX_NETWORKENUM_H
#define FLOATMAX_NETWORKENUM_H
#include <type_traits>
#include <vector>
#include <functional>

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

private:
    std::vector<std::function<void()>> observers;
    enum_type value;
};

#endif //FLOATMAX_NETWORKENUM_H