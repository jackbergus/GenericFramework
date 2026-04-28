// UDPBiDirectional.h
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
// Created by Giacomo Bergami, PhD on 28/04/2026.
//

#ifndef GENERALFRAMEWORK_UDPBIDIRECTIONAL_H
#define GENERALFRAMEWORK_UDPBIDIRECTIONAL_H

#include <jackbergus/networking/udp/UDPClient.h>
#include <jackbergus/networking/udp/UDPServer.h>

template
<typename signal_type>
class UDPBiDirectional {
    const std::string target_ip, source_ip;
    int target_port, source_port;
    UDPClient<signal_type>* client;
    UDPServer<signal_type>* server;
    bool do_wait_;
    unsigned long sec_wait_;
    unsigned long usec_wait_;

public:
    UDPBiDirectional(const std::string &target_ip, const std::string &source_ip, int target_port, int source_port, bool doWait = false, unsigned long sec_wait = 0, unsigned long usec_wait = 10)
        : target_ip(target_ip),
          source_ip(source_ip),
          target_port(target_port),
          source_port(source_port), do_wait_(doWait), sec_wait_(sec_wait), usec_wait_(usec_wait) {
        client = UDPClient<signal_type>::instance(target_ip, target_port);
        server = UDPServer<signal_type>::instance(source_ip, source_port, doWait, sec_wait, usec_wait);
    }

    bool send(const signal_type& signal) {
        if (client)
            return client->send_signal(signal);
        else
            return false;
    }

    std::vector<signal_type> recv(bool clearPreviousSignal = true) {
        if (server) {
            if (clearPreviousSignal) {
                server->clearSignals();
            }
            bool hasReceivedSignal = false;
            while (server->recv_signal()) {
                hasReceivedSignal = true;
            }
            if (hasReceivedSignal) {
                return server->getActiveSignals();
            } else {
                return {};
            }
        } else {
            return {};
        }
    }

    void close() {
        if (client) {
            client->close();
            delete client;
            client = nullptr;
        }
        if (server) {
            server->close();
            delete server;
            server = nullptr;
        }
    }



    ~UDPBiDirectional() {
        close();
    }
};

#endif //GENERALFRAMEWORK_UDPBIDIRECTIONAL_H