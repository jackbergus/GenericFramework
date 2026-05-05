// UDPClient.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - Giacomo Bergami
//
// GeneralFramework is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  GeneralFramework is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GeneralFramework. If not, see <http://www.gnu.org/licenses/>.
//
// Created by Giacomo Bergami, PhD on 26/03/26.
//

#ifndef GENERALFRAMEWORK_UDPCLIENT_H
#define GENERALFRAMEWORK_UDPCLIENT_H

#include <jackbergus/networking/udp/udp.h>
#include <string>
#include <iostream>
#include <vector>

template
<typename signal_type>
class UDPClient {
    std::string server_ip;
    int         server_port;
#ifndef USE_ZMQ
    struct sockaddr_in servaddr;
#endif
    int         rc;
    void *ctx{nullptr};
    void *radio{nullptr};
    std::string mcast_group;

    UDPClient(const std::string& server_ip, int server_port) : server_ip(server_ip), server_port(server_port) {
#ifndef USE_ZMQ
        rc = socket(AF_INET, SOCK_DGRAM, 0);
        memset(&servaddr, 0, sizeof(servaddr));
        // Fill server address info
        servaddr.sin_family = AF_INET;              // IPv4
        servaddr.sin_port   = htons(server_port);          // Server port
        servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // Server IP
#else
        ctx = zmq_ctx_new();
        radio = zmq_socket(ctx, ZMQ_RADIO);
        mcast_group = "udp://" + server_ip + ":" + std::to_string(server_port);
        rc = zmq_connect(radio, mcast_group.c_str());
#endif
    }

public:

    UDPClient(const UDPClient&) = delete;
    UDPClient& operator=(const UDPClient&) = delete;
    UDPClient(UDPClient&&) = delete;
    UDPClient& operator=(UDPClient&&) = delete;

    // static constexpr uint64_t MAX_VAL = (1 << magic_enum::detail::range_max<signal_type>::value);
    static UDPClient* instance(const std::string& server_ip, int server_port) {
        if (!InitNetworking::getInstance()->good())
            return nullptr;
        return new UDPClient(server_ip, server_port);
    }

    bool send_signal(const signal_type& signal) {
#ifdef USE_ZMQ
        zmq_msg_t msg;
        zmq_msg_init_data (&msg, ( void *)&signal, sizeof(signal), NULL, NULL);
        zmq_msg_set_group(&msg, MCAST_GROUP);
        return zmq_sendmsg(radio, &msg, 0) == sizeof(signal);
#else
        if (rc < 0) {
            return false;
        }
        // printf( "Sending message %d\n", message_num);
        auto val= (sendto(rc, (const char*)&signal, sizeof(signal_type), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr)) == sizeof(signal_type));
        if (!val) {
            // std::cout << strerror(errno) << std::endl;
        }
        return val;
#endif
    }

    void close() {
#ifdef USE_ZMQ
        if (radio) {
            zmq_close(radio);
            radio = nullptr;
        }
        if (ctx) {
            zmq_ctx_term(ctx);
            ctx = nullptr;
        }
#else
        // Close socket
        if (rc>=0) {
#if defined(WIN32)||defined(WIN64)
            closesocket(rc);
#else
            ::close(rc);
#endif
            rc = -1;
        }
#endif
    }

    ~UDPClient() {
        close();
    }
};



#endif //GENERALFRAMEWORK_UDPCLIENT_H