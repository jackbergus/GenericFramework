//
// Created by Giacomo Bergami, PhD on 28/04/2026.
//

#define ZMQ_BUILD_DRAFT_API
#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <jackbergus/networking/udp/udp.h>

#if 1
#include "udp_signals_example.h"
#include <jackbergus/networking/udp/UDPBiDirectional.h>
#include "magic_enum/magic_enum.hpp"
#if defined(WIN32)||defined(WIN64)
#include <windows.h>
static inline void sleep(uint64_t secs) {
    Sleep(secs*1000);
}
#else
#include <sys/unistd.h>
#endif


int main(void) {
    std::cout<< "udp2" << std::endl;
    UDPBiDirectional<udp_example_signals> udp2("127.0.0.1", "127.0.0.1", 40000, 40001, false);
    sleep(10);
    for (const auto& val : udp2.recv(true)) {
        std::cout << magic_enum::enum_name(val).data() << std::endl;
    }
    std::cout<< "sending[WORLD,MONDO,CIAO]" << std::endl;
    auto val1 = udp2.send(WORLD);
    auto val2 = udp2.send(MONDO);
    auto val3 = udp2.send(CIAO);
    udp2.close();
    return 0;

}
#else
void my_free (void *data, void *hint)
{
    free (data);
}
const char *unicast_url = "udp://127.0.0.1:40000";
int main() {

    void *ctx = zmq_ctx_new();
    void *radio = zmq_socket(ctx, ZMQ_RADIO);

    const char* url = unicast_url;


    printf( "Connecting radio\n" );
    int rc = zmq_connect(radio, url);
    assert(rc > -1);

    //signal(SIGINT, inthand);
    int message_num = 0;
    for (int i = 0; i<6; i++){
        char *data = (char*)malloc(255);
        memset(data, 0, sizeof(data));
        sprintf(data, "%d", message_num);

        zmq_msg_t msg;
        zmq_msg_init_data (&msg, data, 255, my_free, NULL);
        zmq_msg_set_group(&msg, MCAST_GROUP);

        printf( "Sending message %d\n", message_num);
        zmq_sendmsg(radio, &msg, 0);

        message_num++;
        //usleep(100);
    }

    printf( "Closing");
    zmq_close(radio);
    zmq_ctx_term(ctx);
    return 0;
}
#endif