//
// Created by Giacomo Bergami, PhD on 13/05/2026.
//


#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoringWithSharedFile.h>

#include <magic_enum/magic_enum.hpp>

#include "admissible_nested_example.h"
#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h>
#include <jackbergus/framework/monitor/AnyStructMonitoring.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>

#include <jackbergus/framework/monitor/serializer/binary_to_csv_serializer.h>

#include <jackbergus/framework/monitor/deserializer/FileLogger.h>

#include "jackbergus/data_structures/IntervalTree.h"

void testing() {
    uint64_t counting = 0;
    std::cout << "[" << counting << ", ";

    counting += sizeof(uint32_t)*8;
    std::cout << counting-1 << "]" << std::endl;
    std::cout << "[" << counting << ", ";

    counting += sizeof(uint32_t)*8;
    std::cout << counting-1 << "]" << std::endl;
    std::cout << "[" << counting << ", ";

    counting += sizeof(int16_t)*8;
    std::cout << counting-1 << "]" << std::endl;
    std::cout << "[" << counting << ", ";

    counting += sizeof(int16_t)*8;
    std::cout << counting-1 << "]" << std::endl;
    std::cout << "[" << counting << ", ";

    for (uint64_t j = 0; j<10; j++) {
        counting += sizeof(int16_t)*8;
        std::cout << counting-1 << "]" << std::endl;
        std::cout << "[" << counting << ", ";

        counting += sizeof(int16_t)*8;
        std::cout << counting-1 << "]" << std::endl;
        std::cout << "[" << counting << ", ";

        for (uint64_t i = 0; i<3; i++) {
            counting += 3;
            std::cout << counting-1 << "]" << std::endl;
            std::cout << "[" << counting << ", ";

            counting += 1;
            std::cout << counting-1 << "]" << std::endl;
            std::cout << "[" << counting << ", ";

            counting += 3;
            std::cout << counting-1 << "]" << std::endl;
            std::cout << "[" << counting << ", ";

            counting +=5;
            std::cout << counting-1 << "]" << std::endl;
            std::cout << "[" << counting << ", ";

            counting +=4;
            std::cout << counting-1 << "]" << std::endl;
            std::cout << "[" << counting << ", ";
        }
    }

}


int main() {
    testing();
    std::cout << std::endl << std::endl << std::endl;

    Final_F final_n, elementi_2;
    memset(&final_n, 0, sizeof(final_n));
    final_n.enumerato = 7; // Valori dal tempo 2
    final_n.third = 86; // Valori dal tempo 2
    final_n.first.cho = 5; // Valori dal tempo 4
    final_n.first.jes = 6; // Valori dal tempo 4
    final_n.first.val = 7; // Valori dal tempo 4
    final_n.second[3].val[2].val2 = 3;
    final_n.second[3].val[1].val3 = 5;
    arbitrary_bitset<refl::descriptor::bit_val<Final_F>()> wrapper1((uint64_t *) &final_n);
    std::cout << wrapper1.toString() << std::endl;

    std::cout << sizeof(Element1_N) + sizeof(Element2_L)*10 << std::endl;
    //std::unordered_map<std::string, uint64_t> debug_fieldname_to_vectoroffset;
    memcpy(&elementi_2, &final_n, sizeof(Final_F));
    elementi_2.enumerato3 = 1; // Valori dal tempo 2
    elementi_2.second[7].cho = 19; // Valori dal tempo 7
    elementi_2.second[6].cho = 13; // Valori dal tempo 6
    elementi_2.second[5].cho = 11; // Valori dal tempo 5
    elementi_2.second[3].val[0].filling = 1;
    elementi_2.enumerato2 = 3; // Valori dal tempo 2
    elementi_2.second[3].val[0].val2 = 1;
    elementi_2.second[3].val[0].ripping = 1;
    elementi_2.first.voi_ = 8; // Valori dal tempo 8
    elementi_2.second[3].val[1].val3 = 6;
    elementi_2.second[3].val[2].val2 = 2;

    arbitrary_bitset<refl::descriptor::bit_val<Final_F>()> wrapper2((uint64_t *) &elementi_2);
    std::cout << wrapper2.toString() << std::endl;

    std::shared_ptr<jackbergus::framework::FileSerializer<>> NO_FILE{nullptr};
    auto fields = getNativeType2<Final_F>(NO_FILE, 0, 0);
    IntervalTree<uint64_t, uint64_t> interval_of_offsets;
    for (uint64_t idx = 0, N = fields.size(); idx < N; idx++) {
        const auto& field = fields[idx];
        std::cout << "[" << field.bitOffset() << ", " << field.bitOffset()+field.bitSize()-1 << "] for " << field.field_name() <<  std::endl;
        interval_of_offsets.insertInterval({field.bitOffset(), field.bitOffset()+field.bitSize()-1, idx});
        //debug_fieldname_to_vectoroffset[field.field_name()] = idx;
    }

    std::vector<stack_function> stacks;
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Basically, getNativeType2 operates like such:
    // -- At each iteration, we insert an element of the stack like such, that allows to navigate the structure from the beginning of it
    // -- As a consequence, we have that we can navigate from the root object towards the field, by only knowin which bit
    // -- within the data structure was modified and, therefore, which inner field was changed
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    auto differences = wrapper1.deltaFromIntervalTreeSlot(interval_of_offsets, wrapper2);
    std::cout << "We found #" << differences.size() << " changes, which consist of the following: "  << std::endl;
    for (const auto& idx : differences) {
        lightweight_any finale_wrapped{&elementi_2};
        auto& field = fields[idx];
        const auto& s = field.getStack();
        for (auto it = s.rbegin(); it != s.rend(); it++) {
            finale_wrapped = (*it)(finale_wrapped);
        }

        std::cout << field.field_name() << ": changed as ";
        field.printNative(std::cout, finale_wrapped) << std::endl;
    }

    // stacks.emplace_back([](auto finale_wrapped) {
    //     auto ptr1 = (Final_F*)finale_wrapped.raw();
    //     auto& ref_field2 = *ptr1.*(refl::trait::get_t<1, refl::member_list<Final_F>>::pointer);
    //     lightweight_any field_access1{&ref_field2[3]};
    //     return field_access1;
    // });
    // stacks.emplace_back([](auto field_access1) {
    //     auto ptr2 = (Element2_N*)field_access1.raw();
    //     auto& ref_field3 = *ptr2.*(refl::trait::get_t<2, refl::member_list<Element2_N>>::pointer);
    //     lightweight_any field_access2{&ref_field3[2]};
    //     return field_access2;
    // });
    // stacks.emplace_back([](auto field_access2) {
    //     auto ref_field3 = getter<InnerNestingLevel, 2>(*(InnerNestingLevel*)field_access2.raw());
    //     return lightweight_any{ref_field3};
    // });

}
