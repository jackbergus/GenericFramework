//
// Created by Giacomo Bergami, PhD on 11/05/2026.
//

#include <jackbergus/algorithms/ExternalMergeSort.h>

void quicksort_works() {
        std::vector<jackbergus::framework::new_delta_data_structure> vec;
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(210));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(10));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(20));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(3));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(4));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(2));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(1));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(0));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(50));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(55));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(21));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(22));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(11));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(12));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(23));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(22));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(21));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(20));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(51));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(52));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(53));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(54));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(211));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(212));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(500));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(200));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(201));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(202));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(213));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(214));
    vec.push_back(jackbergus::framework::new_delta_data_structure::newTimestampRecord(215));

    auto F= [](const jackbergus::framework::new_delta_data_structure& x ) { return x.timestamp; };
    QuickSort<jackbergus::framework::new_delta_data_structure, double>(vec, F);
    for (const auto& ref : vec)
        std::cout << ref.timestamp << std::endl;
}

int main(void) {

    {
        // Reading the binary file, as per previous serialization
        jackbergus::framework::FileBlockReader<> fbr;
        // Buffer containing one page of memory per entry
        jackbergus::framework::FileBlockWrapper<> buffer;
        std::vector<jackbergus::framework::new_delta_data_structure> Q;
        fbr.open("some_binary_file.bin");
        while (fbr.read(buffer, false)) {
            for (uint64_t i = 0, N = buffer.size(); i < N; i++) {
                auto ptr = buffer.getNewRecord(i);
                Q.emplace_back(*ptr);
                //
            }
        }

        QuickSort<jackbergus::framework::new_delta_data_structure, std::tuple<double, uint8_t, uint8_t>>(Q, [](const auto& x) {return std::tie(x.timestamp, x.structure_id, x.unnested_field_id);});
        for (const auto& ref : Q) {
            std::cout << magic_enum::enum_name(ref.actual_type).data() << " for " << (uint64_t)ref.structure_id << " of " << (uint64_t)ref.unnested_field_id << " @ " << ref.timestamp << std::endl;
        }
    }

    {
        ExternalMergeSort<> test("some_binary_file.bin");
        test.start()   ;
    }


    // Reading the binary file, as per previous serialization
    jackbergus::framework::FileBlockReader<> fbr;
    // Buffer containing one page of memory per entry
    jackbergus::framework::FileBlockWrapper<> buffer;
    fbr.open("some_binary_file.bin");
    while (fbr.read(buffer, false)) {
        for (uint64_t i = 0, N = buffer.size(); i < N; i++) {
            auto ptr = buffer.getNewRecord(i);
            std::cout << magic_enum::enum_name(ptr->actual_type).data() << " for " << (uint64_t)ptr->structure_id << " of " << (uint64_t)ptr->unnested_field_id << " @ " << ptr->timestamp << std::endl;
        }
    }

    return 0;
}
