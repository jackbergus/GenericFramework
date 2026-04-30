//
// Created by Giacomo Bergami, PhD on 28/04/2026.
//

#include "udp_signals_example.h"
#include <jackbergus/networking/udp/UDPBiDirectional.h>
#include "magic_enum/magic_enum.hpp"
#if defined(WIN32)||defined(WIN64)
#include <windows.h>
static inline void sleep(uint64_t secs) {
    Sleep(secs*1000);
}
#endif


int main(void) {
    UDPBiDirectional<udp_example_signals> udp2("127.0.0.1", "127.0.0.1", 6000, 8000, false);
    sleep(10);
    for (const auto& val : udp2.recv(true)) {
        std::cout << magic_enum::enum_name(val).data() << std::endl;
    }
    udp2.send(WORLD);
    udp2.send(MONDO);
    udp2.send(CIAO);
    udp2.close();

}
