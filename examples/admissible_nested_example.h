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
} __attribute__((packed));




#include <limits>

constexpr auto uint32M = std::numeric_limits<uint32_t>::max();
constexpr auto uint8M = std::numeric_limits<uint8_t>::max();
constexpr auto int16M = std::numeric_limits<int16_t>::max();

struct InnerNestingLevel {
    int16_t ripping : 3;
    int16_t val1 : 1;
    int16_t val2 : 3;
    int16_t val3 : 5;
    int16_t filling: 4;
} __attribute__((packed));
static_assert(sizeof(InnerNestingLevel) == 2);

struct Element2_N {
    int16_t cho;
    int16_t voi_;
    InnerNestingLevel val[3];
} __attribute__((packed));
static_assert(sizeof(Element2_N) == sizeof(int16_t)*2+ 3*sizeof(InnerNestingLevel));


struct Final_N {
    Element1_N first;
    Element2_N second[10];
    uint32_t third;
    uint8_t enumerato;
} __attribute__((packed));

struct Final_F {
    Element1_N first;
    Element2_N second[10];
    uint32_t third;
    uint8_t enumerato : 3;
    uint8_t enumerato2 : 3;
    uint8_t enumerato3 : 2;
} __attribute__((packed));

namespace std {
    template<>
    struct numeric_limits<InnerNestingLevel> {
        constexpr static InnerNestingLevel min() {
            return {0,0,0,0, 0};
        }
        constexpr static InnerNestingLevel max() {
            return {7,1,7,31, 15};
        }
    };

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
            constexpr auto snd = numeric_limits<InnerNestingLevel>::min();
            return {0,0,{snd, snd, snd}};
        }
        constexpr static Element2_N max() {
            constexpr auto snd = numeric_limits<InnerNestingLevel>::max();
            return {int16M,int16M, {snd, snd, snd}};
        }
    };

    template<>
struct numeric_limits<Final_N> {
        constexpr static Final_N min() {
            constexpr auto snd = numeric_limits<Element2_N>::min();
            return {numeric_limits<Element1_N>::min(),{snd,snd,snd,snd,snd,snd,snd,snd,snd,snd},  0,0};
        }
        constexpr static Final_N max() {
            constexpr auto snd = numeric_limits<Element2_N>::max();
            return {numeric_limits<Element1_N>::max(), {snd,snd,snd,snd,snd,snd,snd,snd,snd,snd}, uint32M, uint8M};
        }
    };

    template<>
struct numeric_limits<Final_F> {
        constexpr static Final_F min() {
            constexpr auto snd = numeric_limits<Element2_N>::min();
            return {numeric_limits<Element1_N>::min(),{snd,snd,snd,snd,snd,snd,snd,snd,snd,snd},  0,0, 0, 0};
        }
        constexpr static Final_F max() {
            constexpr auto snd = numeric_limits<Element2_N>::max();
            return {numeric_limits<Element1_N>::max(), {snd,snd,snd,snd,snd,snd,snd,snd,snd,snd}, uint32M, 7, 7, 3};
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
REFL_AUTO(type(InnerNestingLevel), bitfield(ripping, 3), bitfield(val1, 1), bitfield(val2, 3), bitfield(val3, 5), bitfield(filling, 4));
REFL_AUTO(type(Element1_N), field(val), field(jes), field(cho), field(voi_))
REFL_AUTO(type(Element2_N), field(cho), field(voi_), field(val))
REFL_AUTO(type(Final_N), field(first), field(second), field(third), field(enumerato))
REFL_AUTO(type(Final_F), field(first), field(second), field(third), bitfield(enumerato, 3), bitfield(enumerato2, 3), bitfield(enumerato3, 2))
REFL_AUTO(type(Element1_L), sfield(val), sfield(jes), sfield(cho), sfield(voi_))
REFL_AUTO(type(Element2_L), sfield(cho), sfield(voi_), sfield(val))
REFL_AUTO(type(Final_L), sfield(first), sfield(second), sfield(third), sfield(objectivo))
static_assert(refl::descriptor::bit_val<Element1_N>() == sizeof(Element1_N)*8);
static_assert(refl::descriptor::bit_val<InnerNestingLevel>() == sizeof(InnerNestingLevel)*8);
static_assert(refl::descriptor::bit_val<Element2_N>() == sizeof(Element2_N)*8);
static_assert(refl::descriptor::bit_val<Final_N>() == sizeof(Final_N)*8);
static_assert(refl::descriptor::bit_val<Final_F>() == sizeof(Final_F)*8);

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