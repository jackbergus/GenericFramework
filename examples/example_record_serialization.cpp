#include <iostream>

#include <random>

#include <jackbergus/framework/ndp/AnyStructMonitoring.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>

struct ExampleStructure {
    uint64_t a, b, c, d, e;
};
REFL_AUTO(type(ExampleStructure), field(a), field(b), field(c), field(d), field(e))

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    std::random_device rd; // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<uint64_t> dista(10, 55); // Inclusive range

    jackbergus::framework::AnyStructMonitoring<ExampleStructure> obj(0);
    obj.setFile("NitakungodeaMilele");
    ExampleStructure stru;
    for (uint64_t i = 0; i<100; i++) {
        stru.a = dista(gen);
        std::cout << stru.a << std::endl;
        stru.b = dista(gen);
        stru.c = dista(gen);
        stru.d = dista(gen);
        stru.e = dista(gen);
        obj.updateValue(i, stru);
    }

    jackbergus::framework::FileBlockReader<> flip("NitakungodeaMilele_a[T_U_INTEGRAL[8]].bin");
    jackbergus::framework::FileBlockWrapper<> block;
    uint64_t total = 0;
    while (flip.read(block)) {
        total += block.size();
        for (uint64_t i = 0, N = block.size(); i < N; ++i) {
            auto cp = block.get(i);
            std::cout << *((uint64_t*)cp.second) << "@ [" << cp.first->start << ", " << cp.first->end << "]" << std::endl;
        }
    }

    return 0;
}