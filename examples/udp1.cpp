//
// Created by Giacomo Bergami, PhD on 28/04/2026.
//

#include <jackbergus/networking/udp/udp.h>

#if 1
#include <magic_enum/magic_enum.hpp>
#include "udp_signals_example.h"
#include <jackbergus/networking/udp/UDPBiDirectional.h>

#if defined(WIN32)||defined(WIN64)
#include <windows.h>
static inline void sleep(uint64_t secs) {
    Sleep(secs*1000);
}
#else
#include <sys/unistd.h>
#endif

int main(void) {
    auto ptr = InitNetworking::getInstance();
    UDPBiDirectional<udp_example_signals> udp1("127.0.0.1", "127.0.0.1", 8001, 8000, false);
    std::cout<< "udp1 sending[HELLO,MONDO x 2]" << std::endl;
    auto val1 = udp1.send(HELLO);
    auto val2 = udp1.send(MONDO);
    auto val3 = udp1.send(MONDO);
    sleep(20);
    for (const auto& val : udp1.recv(true)) {
        std::cout << magic_enum::enum_name(val).data() << std::endl;
    }
    udp1.close();
    ptr->close();
    return 0;
}
#else
const char *mcast_url = "udp://239.0.0.1:40000";
const char *unicast_url = "udp://127.0.0.1:40000";
int main() {
    void *ctx = zmq_ctx_new();
    void *dish = zmq_socket(ctx, ZMQ_DISH);

    const char* url = NULL;
    url = unicast_url;

    printf( "Binding dish\n" );
    int rc = zmq_bind(dish, url);
    assert(rc > -1);

    // const char* group = "mcast_test";
    printf( "Dish joining group\n" );
    zmq_join(dish, MCAST_GROUP);

    //signal(SIGINT, inthand);

    printf( "Waiting for dish message\n" );
    for (int i = 0; i<6; i++){
        zmq_msg_t recv_msg;
        zmq_msg_init (&recv_msg);
        while (zmq_recvmsg (dish, &recv_msg, ZMQ_DONTWAIT ) == -1);
        printf("%s", (char*)zmq_msg_data(&recv_msg));
        printf( "\n" );

        zmq_msg_close (&recv_msg);
    }

    printf( "Closing");
    zmq_close(dish);
    zmq_ctx_term(ctx);

    return 0;
}
#endif