//
// Created by gyankos on 04/05/26.
//

#include <iostream>
#include <refl.hpp>

struct Element1_N {
    uint32_t val;
    uint32_t jes;
    uint8_t cho : 2;
    uint8_t voi_ : 3;
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
            return {uint32M,uint32M,3,7};
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

// #include "jackbergus/data/NetworkFloat.h"
// #include "jackbergus/data/NetworkInt.h"
// #include "jackbergus/data/NetworkEnum.h"
//
// FLTTYPE(FMuxThou, 1000)
//
// enum class Enumo : uint8_t {
//     EnumoNone = 0,
//     UIN = 1,
//     UBIZ = 3,
//     UJOZ = 5
// };
//
// using BasicFloat = Float<int32_t, FZero, FMuxThou, FEps, FMuxThou>;
// using BasicS16Int = Int<int16_t, 0, 1000>;
// using BasicS32Int = Int<>;
// using BasicU32Int = Int<uint32_t, 1, 1000>;
// using BasicEnumo = Enum<Enumo, Enumo::EnumoNone>;
//
// struct Element1_L {
//     BasicFloat val;
//     BasicFloat jes;
//     BasicS16Int cho;
//     BasicS16Int voi_;
// };
//
// struct Element2_L {
//     BasicS16Int cho;
//     BasicS16Int voi_;
//     BasicU32Int val;
// };
//
// struct Final_L {
//     Element1_L first;
//     std::array<Element2_L,10> second;
//     BasicU32Int third;
//     BasicEnumo objectivo;
// };
// static_assert(sizeof(std::array<Element2_L,10>) == sizeof(Element2_L[10]));

// In C++17, this is the best that you can do... This is entirely avoidable in C++20
#include "refl.hpp"
REFL_AUTO(type(Element1_N), field(val), field(jes), bitfield(cho), bitfield(voi_))
REFL_AUTO(type(Element2_N), field(cho), field(voi_), field(val))
REFL_AUTO(type(Final_N), field(first), field(second), field(third), field(enumerato))

int main(void) {

    Final_N final_n = std::numeric_limits<Final_N>::min();

    // Now, considering nested changes, forsooth!
    final_n.first.val = 7; // Valori dal tempo 4
    final_n.first.jes = 6; // Valori dal tempo 4
    final_n.first.cho = 3; // Valori dal tempo 4
    final_n.first.voi_ = 7; // Valori dal tempo 8
    final_n.second[5].cho = 11; // Valori dal tempo 5
    final_n.second[6].cho = 13; // Valori dal tempo 6
    final_n.second[7].cho = 19; // Valori dal tempo 7
    final_n.enumerato = 123; // Valori dal tempo 2
    final_n.third = 86;      // Valori dal tempo 2

    std::cout << getter<Element1_N, 0>(getter<Final_N, 0>(final_n)) << std::endl;
    std::cout << getter<Element1_N, 1>(getter<Final_N, 0>(final_n)) << std::endl;
    std::cout << getter<Element1_N, 2>(getter<Final_N, 0>(final_n)) << std::endl;
    std::cout << getter<Element1_N, 3>(getter<Final_N, 0>(final_n)) << std::endl;
    std::cout << (uint64_t)getter<Final_N, 2>(final_n) << std::endl;
    std::cout << (uint64_t)getter<Final_N, 3>(final_n) << std::endl;

    return 0;
}