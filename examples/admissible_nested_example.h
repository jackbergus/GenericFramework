//
// Created by mbda on 27/04/2026.
//

#ifndef GENERALFRAMEWORK_ADMISSIBLE_NESTED_EXAMPLE_H
#define GENERALFRAMEWORK_ADMISSIBLE_NESTED_EXAMPLE_H

#include <iostream>
#include <cstdint>
#include <array>

struct Element1_N {
    uint32_t val;
    uint32_t jes;
    int16_t cho;
    int16_t voi_;
};

#include <limits>

constexpr auto uint32M = std::numeric_limits<uint32_t>::max();
constexpr auto uint8M = std::numeric_limits<uint8_t>::max();
constexpr auto int16M = std::numeric_limits<int16_t>::max();



struct Element2_N {
    int16_t cho;
    int16_t voi_;
    uint32_t val;
};



struct Final_N {
    Element1_N first;
    Element2_N second[10];
    uint32_t third;
    uint8_t enumerato;
};

namespace std {
    template<>
    struct numeric_limits<Element1_N> {
        constexpr static Element1_N min() {
            return {0,0,0,0};
        }
        constexpr static Element1_N max() {
            return {uint32M,uint32M,int16M,int16M};
        }
    };

    template<>
struct numeric_limits<Element2_N> {
        constexpr static Element2_N min() {
            return {0,0,0};
        }
        constexpr static Element2_N max() {

            return {int16M,int16M, uint32M};
        }
    };

    template<>
struct numeric_limits<Final_N> {
        constexpr static Final_N min() {
            return {numeric_limits<Element1_N>::min(),0,0};
        }
        constexpr static Final_N max() {
            constexpr auto snd = numeric_limits<Element2_N>::max();
            return {numeric_limits<Element1_N>::max(), {snd,snd,snd,snd,snd,snd,snd,snd,snd,snd}, uint32M, uint8M};
        }
    };
}

#include "jackbergus/data/NetworkFloat.h"
#include "jackbergus/data/NetworkInt.h"
#include "jackbergus/data/NetworkEnum.h"

FLTTYPE(FMuxThou, 1000)

enum class Enumo : uint8_t {
    EnumoNone = 0,
    UIN = 1,
    UBIZ = 3,
    UJOZ = 5
};

using BasicFloat = Float<int32_t, FZero, FMuxThou, FEps, FMuxThou>;
using BasicS16Int = Int<int16_t, 0, 1000>;
using BasicS32Int = Int<>;
using BasicU32Int = Int<uint32_t, 1, 1000>;
using BasicEnumo = Enum<Enumo, Enumo::EnumoNone>;

struct Element1_L {
    BasicFloat val;
    BasicFloat jes;
    BasicS16Int cho;
    BasicS16Int voi_;
};

struct Element2_L {
    BasicS16Int cho;
    BasicS16Int voi_;
    BasicU32Int val;
};

struct Final_L {
    Element1_L first;
    std::array<Element2_L,10> second;
    BasicU32Int third;
    BasicEnumo objectivo;
};
static_assert(sizeof(std::array<Element2_L,10>) == sizeof(Element2_L[10]));

// In C++17, this is the best that you can do... This is entirely avoidable in C++20
#include "refl.hpp"
REFL_AUTO(type(Element1_N), field(val), field(jes), field(cho), field(voi_))
REFL_AUTO(type(Element2_N), field(cho), field(voi_), field(val))
REFL_AUTO(type(Final_N), field(first), field(second), field(third), field(enumerato))
REFL_AUTO(type(Element1_L), field(val), field(jes), field(cho), field(voi_))
REFL_AUTO(type(Element2_L), field(cho), field(voi_), field(val))
REFL_AUTO(type(Final_L), field(first), field(second), field(third), field(objectivo))

enum testing_enums_for_first_time : uint8_t {
    VAL_0 = 0,
    VAL_1 = 1,
    VAL_2 = 2,
};

struct BogusConcurrentDataRecord {
    uint64_t                        val;
    double                          timestamp;
    testing_enums_for_first_time    t;
};

REFL_AUTO(type(BogusConcurrentDataRecord), field(val), field(timestamp), field(t))


#endif //GENERALFRAMEWORK_ADMISSIBLE_NESTED_EXAMPLE_H