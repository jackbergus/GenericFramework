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
    }

    auto result = wrapper2.deltaFromIntervalTreeSlot(interval_of_offsets, wrapper1);
    for (const auto& idx : result) {
        std::cout << " -- " << fields[idx].field_name() << std::endl;
    }

    lightweight_any finale_wrapped{&final_n};
    auto ptr = (Final_F*)finale_wrapped.raw();
    auto& ref_field2 = *ptr.*(refl::trait::get_t<1, refl::member_list<Final_F>>::pointer);
    ref_field2[1];


}
