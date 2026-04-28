//
// Created by Giacomo Bergami, PhD on 28/04/2026.
//

#include "udp_signals_example.h"
#include <jackbergus/networking/udp/UDPBiDirectional.h>

#include "magic_enum/magic_enum.hpp"

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
