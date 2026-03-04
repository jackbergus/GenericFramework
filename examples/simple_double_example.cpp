//
// Created by gyankos on 04/03/26.
//

#include <cassert>
#include <iostream>
#include <random>

#include "jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h"
#include "jackbergus/framework/monitor/ContinuousVariableMonitoring.h"
#include "jackbergus/framework/ndp/FileBlockReader.h"

int main() {
    // auto s = getNativeType<ExampleStructure>(0);
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the <b>lang</b> variable name to see how CLion can help you rename it.
    double min = 10.0;
    double max = 50.0;
    jackbergus::framework::AnyFundamentalVariableMonitoring counter = flatten_type_to_enum<double>(0, 0, "aDouble");
    jackbergus::framework::ContinuousVariableMonitoring<double> cmp{0};
    auto binary = "binary.bin";
    counter.setFile(binary);

    std::random_device rd; // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> dista(min, max); // Inclusive range
    std::uniform_int_distribution<int> dist(0, 1);

    double opaque = 0.0, transparent = 0.0;
    constexpr uint64_t MAX = 10000;

    for (uint64_t idx = 0; idx < MAX; ++idx) {
        bool randomBool = static_cast<bool>(dist(gen));
        auto randomNum = dista(gen);

        {
            auto start = std::chrono::steady_clock::now();
            if (randomBool) {
                std::cout << idx << ": "  << randomNum << std::endl;
                counter.updateValue(idx, randomNum);
                assert(*counter.get<double>() == randomNum);
            } else {
                std::cout << idx  << " Invalid " << std::endl;
                counter.setInvalidValue(idx);
            }
            auto end = std::chrono::steady_clock::now();
            opaque += std::chrono::duration<double, std::milli>(end - start).count();
        }

        {
            auto start = std::chrono::steady_clock::now();
            if (randomBool) {
                std::cout << idx << ": "  << randomNum << std::endl;
                cmp.updateValue(idx, randomNum);
                assert(*counter.get<double>() == randomNum);
            } else {
                std::cout << idx  << " Invalid " << std::endl;
                cmp.setInvalidValue(idx);
            }
            auto end = std::chrono::steady_clock::now();
            transparent += std::chrono::duration<double, std::milli>(end - start).count();
        }


    }
    counter.clearFile();

    std::cout << ((opaque)/((double)MAX)) << " vs. " << ((transparent)/((double)MAX))  <<std::endl;
    jackbergus::framework::FileBlockReader<> flip(binary);
    jackbergus::framework::FileBlockWrapper<> block;
    uint64_t total = 0;
    while (flip.read(block)) {
        total += block.size();
        for (uint64_t i = 0, N = block.size(); i < N; ++i) {
            auto cp = block.get(i);
            std::cout << *((double*)cp.second) << "@ [" << cp.first->start << ", " << cp.first->end << "]" << std::endl;
        }
    }

    // FILE* filp = fopen(binary, "rb" );
    // if (!filp) { printf("Error: could not open file %s\n", binary); return -1; }
    //
    // char * buffer[1024];
    // int bytes;
    // memset(buffer, 0, sizeof(buffer));
    // uint64_t total = 0;
    // while ( (bytes = fread(buffer, sizeof(char), 1024, filp)) > 0 ) {
    //     if (bytes == 1024) {
    //         uint64_t N_elements_in_block = *(uint64_t *) buffer;
    //         total += N_elements_in_block;
    //         jackbergus::framework::BlockHeader* first_header = (jackbergus::framework::BlockHeader*)(((uint64_t *) buffer)+1);
    //         for (uint64_t i = 0; i < N_elements_in_block; ++i) {
    //             double val = *(double*)(((char*)first_header)+sizeof(jackbergus::framework::BlockHeader));
    //             std::cout << val << "@ [" << first_header->start << ", " << first_header->end << "]" << std::endl;
    //             first_header = (jackbergus::framework::BlockHeader*)(((char*)first_header)+sizeof(jackbergus::framework::BlockHeader)+first_header->payload_size);
    //         }
    //
    //     }
    //     memset(buffer, 0, sizeof(buffer));
    // }
    //         std::cout << total << std::endl;

    // Done and close.
    // fclose(filp);

    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}
