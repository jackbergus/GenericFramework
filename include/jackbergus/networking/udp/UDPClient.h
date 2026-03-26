//
// Created by gyankos on 26/03/26.
//

#ifndef GENERALFRAMEWORK_UDPCLIENT_H
#define GENERALFRAMEWORK_UDPCLIENT_H

#include <jackbergus/networking/udp/udp.h>

template
<typename signal_type>
class UDPClient {
    std::string server_ip;
    int         server_port;
    struct sockaddr_in servaddr;
    int         sockfd;

    UDPClient(const std::string& ip, int port, int socket) : server_ip(ip), server_port(port), sockfd(socket) {
        memset(&servaddr, 0, sizeof(servaddr));
        // Fill server address info
        servaddr.sin_family = AF_INET;              // IPv4
        servaddr.sin_port   = htons(port);          // Server port
        servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // Server IP
    }

public:
    // static constexpr uint64_t MAX_VAL = (1 << magic_enum::detail::range_max<signal_type>::value);
    static UDPClient* instance(const std::string& ip, int port) {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            return nullptr;
        }
        return new UDPClient(ip, port, sockfd);
    }

    bool send_signal(const signal_type& signal) {
        return (sendto(sockfd, (const void*)&signal, sizeof(signal_type), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr)) == sizeof(signal_type));
    }

    void close() {
        // Close socket
        if (sockfd>=0) {
            close(sockfd);
            sockfd = -1;
        }
    }

    ~UDPClient() {
        close();
    }
};



#endif //GENERALFRAMEWORK_UDPCLIENT_H