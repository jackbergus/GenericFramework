#include <iostream>
#include <cstdint>
#include <array>
#include "jackbergus/data/template_typing.h"

struct Element1_N {
    uint32_t val;
    uint32_t jes;
    int16_t cho;
    int16_t voi_;
};

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

#include "jackbergus/data/recursive_encoder.h"

int main() {
    Final_L f_src;
    Final_L f_dst;
    Final_N f_swap, f_swap2;

    // constexpr auto val = get_field2<Elements, 0>::name;
    // static_assert(std::is_same_v<get_field_type<Elements, 0>, int>);
    static_assert(std::is_base_of<_enum, BasicEnumo>::value);
    static_assert(is_same<uint8_t, BasicEnumo::T>::value);
    std::vector<std::function<void()>> elementi;

    encode_extended(&f_swap, f_src);
    encode_extended(&f_swap2, f_src);
    // f_src.second.fill({});
    update(f_src);

    // Now, attempting to change the raw by just setting the logical!
    f_src.third = 133;
    f_src.objectivo  = Enumo::UJOZ;
    f_src.second[3].cho = 42;
    f_src.second[3].voi_ = 42;

    f_src.first.jes = (float) 1.23;
    f_src.first.val = (float) 1.35;
    f_src.first.cho = 123;
    f_src.first.voi_ = 123;


    // encode(f_swap, f_src);
    decode(f_dst, f_swap);

    // std::cout << encode(network, src) << std::endl;
    // std::cout << decode(dst, network) << std::endl;

    return 0;
}
