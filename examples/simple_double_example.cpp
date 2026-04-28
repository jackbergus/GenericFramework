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
    auto binary_a = "binary_any.bin";
    auto binary_d = "binary_double.bin";
    counter.setFile(binary_a);
    cmp.setFile(binary_d);

    std::random_device rd; // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> dista(min, max); // Inclusive range
    std::uniform_int_distribution<int> dist(0, 1);

    double opaque = 0.0, transparent = 0.0;
    constexpr uint64_t MAX = 50;

    for (uint64_t idx = 0; idx < MAX; ++idx) {
        bool randomBool = true; //static_cast<bool>(dist(gen));
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
    cmp.clearFile();

    std::cout << ((opaque)/((double)MAX)) << " vs. " << ((transparent)/((double)MAX))  <<std::endl;

    {
        jackbergus::framework::FileBlockReader<> flip(binary_a);
        jackbergus::framework::FileBlockWrapper<> block;
        uint64_t total = 0;
        while (flip.read(block)) {
            total += block.size();
            for (uint64_t i = 0, N = block.size(); i < N; ++i) {
                auto cp = block.get(i);
                std::cout << *((double*)cp.second) << "@ [" << cp.first->start << ", " << cp.first->end << "]" << std::endl;
            }
        }
    }
    {
        jackbergus::framework::FileBlockReader<> flip(binary_d);
        jackbergus::framework::FileBlockWrapper<> block;
        uint64_t total = 0;
        while (flip.read(block)) {
            total += block.size();
            for (uint64_t i = 0, N = block.size(); i < N; ++i) {
                auto cp = block.get(i);
                std::cout << *((double*)cp.second) << "@ [" << cp.first->start << ", " << cp.first->end << "]" << std::endl;
            }
        }
    }

}
