
#include "admissible_nested_example.h"
#include "jackbergus/data/UpdatableElement.h"

int main() {
    // Final_L f_src;
    Final_L f_dst;
    Final_N f_swap, f_swap2;

    UpdatableElement<Final_L, Final_N> memory_binder;


    // constexpr auto val = get_field2<Elements, 0>::name;
    // static_assert(std::is_same_v<get_field_type<Elements, 0>, int>);
    static_assert(std::is_base_of<_enum, BasicEnumo>::value);
    static_assert(is_same<uint8_t, BasicEnumo::T>::value);
    std::vector<std::function<void()>> elementi;

    memory_binder.associateWithRawMemory(&f_swap);
    memory_binder.associateWithRawMemory(&f_swap2);

    // encode_extended(&f_swap, f_src);
    // encode_extended(&f_swap2, f_src);
    // f_src.second.fill({});
    memory_binder.forceBroadcast();

    // Now, attempting to change the raw by just setting the logical!
    memory_binder.setBit(0);
    memory_binder.data.third = 133;
    memory_binder.data.objectivo  = Enumo::UJOZ;
    memory_binder.data.second[3].cho = 42;
    memory_binder.data.second[3].voi_ = 42;

    memory_binder.setBit(1);
    memory_binder.data.first.jes = (float) 1.23;
    memory_binder.data.first.val = (float) 1.35;
    memory_binder.data.first.cho = 123;
    memory_binder.clearBits();
    memory_binder.data.first.voi_ = 123;

    memory_binder.serialize_to_csv(std::cout, {"third", "objectivo", "first.cho", "ambara", "bacci", "second[3].cho", "second[3].voi_"});

    // encode(f_swap, f_src);
    // decode(f_dst, f_swap);

    // std::cout << encode(network, src) << std::endl;
    // std::cout << decode(dst, network) << std::endl;

    return 0;
}
