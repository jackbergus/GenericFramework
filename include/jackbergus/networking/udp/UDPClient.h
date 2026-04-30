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
    struct sockaddr_in servaddr;
    int         sockfd;

    UDPClient(const std::string& server_ip, int server_port, int socket) : server_ip(server_ip), server_port(server_port), sockfd(socket) {
        memset(&servaddr, 0, sizeof(servaddr));
        // Fill server address info
        servaddr.sin_family = AF_INET;              // IPv4
        servaddr.sin_port   = htons(server_port);          // Server port
        servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // Server IP
    }

public:
    // static constexpr uint64_t MAX_VAL = (1 << magic_enum::detail::range_max<signal_type>::value);
    static UDPClient* instance(const std::string& server_ip, int server_port) {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            return nullptr;
        }
        return new UDPClient(server_ip, server_port, sockfd);
    }

    bool send_signal(const signal_type& signal) {
        auto val= (sendto(sockfd, (const char*)&signal, sizeof(signal_type), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr)) == sizeof(signal_type));
        if (!val) {
            std::cout << strerror(errno) << std::endl;
        }
        return val;
    }

    void close() {
        // Close socket
        if (sockfd>=0) {
#if defined(WIN32)||defined(WIN64)
            closesocket(sockfd);
#else
            ::close(sockfd);
#endif
            sockfd = -1;
        }
    }

    ~UDPClient() {
        close();
    }
};



#endif //GENERALFRAMEWORK_UDPCLIENT_H