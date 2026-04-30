//
// Created by Giacomo Bergami, PhD on 28/04/2026.
//

#include <magic_enum/magic_enum.hpp>
#include "udp_signals_example.h"
#include <jackbergus/networking/udp/UDPBiDirectional.h>
#if defined(WIN32)||defined(WIN64)
#include <windows.h>
static inline void sleep(uint64_t secs) {
    Sleep(secs*1000);
}
#endif

int main(void) {
    UDPBiDirectional<udp_example_signals> udp1("127.0.0.1", "127.0.0.1", 8000, 6000, false);
    udp1.send(HELLO);
    udp1.send(MONDO);
    udp1.send(MONDO);
    sleep(20);
    for (const auto& val : udp1.recv(true)) {
        std::cout << magic_enum::enum_name(val).data() << std::endl;
    }
    udp1.close();
}