// UDPServer.h
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

#ifndef GENERALFRAMEWORK_UDPSERVER_H
#define GENERALFRAMEWORK_UDPSERVER_H

#include <jackbergus/networking/udp/udp.h>

template
<typename signal_type>
class UDPServer {
  std::string server_ip;
  int         server_port;
  struct sockaddr_in servaddr, cliaddr;
  int         sockfd;

  UDPServer(const std::string& ip, int port, int socket) : server_ip(ip), server_port(port), sockfd(socket) {

  }


public:
  // static constexpr uint64_t MAX_VAL = (1 << magic_enum::detail::range_max<signal_type>::value);

  static UDPServer* instance(const std::string& ip, int port, bool doWait = false, unsigned long sec_wait = 0, unsigned long usec_wait = 10) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
      return nullptr;
    }
    int OptVal = 1;
    auto ris = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
    if (ris == -1)  {
      printf ("setsockopt() SO_REUSEADDR failed, Errno: %d \"%s\"  SOCKET_ERROR=%d\n",
          errno,strerror(errno), GetWSASocketError(sockfd));
      return nullptr;
    }
    if (!doWait) {
      struct timeval read_timeout;
      read_timeout.tv_sec = 0;
      read_timeout.tv_usec = 10;
      ris = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
      if (ris == -1)  {
        printf ("setsockopt() SO_RCVTIMEO failed, Errno: %d \"%s\"  SOCKET_ERROR=%d\n",
            errno,strerror(errno), GetWSASocketError(sockfd));
        return nullptr;
      }
    }
    struct sockaddr_in Local;
    memset(&Local, 0, sizeof(servaddr));
    // Fill server address info
    Local.sin_family = AF_INET;              // IPv4
    Local.sin_port   = htons(port);          // Server port
    Local.sin_addr.s_addr = inet_addr(ip.c_str()); // Server IP
    ris = bind(sockfd, (struct sockaddr*) &Local, sizeof(Local));
    if (ris == -1)  {
      printf ("listen() failed, Err: %d \"%s\"  SOCKET_ERROR=%d\n",
          errno, strerror(errno), GetWSASocketError(sockfd));
      return nullptr;
    }
    return new UDPServer(ip, port, sockfd);
  }

  bool recv_signal(void) {
    socklen_t len;
    int n;
    len = sizeof(cliaddr);  //len is value/result
    signal_type result;
    n = recvfrom(sockfd, (char *)&result, sizeof(signal_type),
                0, ( struct sockaddr *) &cliaddr,
                &len);
    if (n==sizeof(result)) {
      bitset_.insert(result);
      return true;

    } else {
      std::cout << strerror(errno) << std::endl;
      return false;
    }
  }

  std::vector<signal_type> getActiveSignals() {
    std::vector<signal_type> signals{bitset_.begin(), bitset_.end()};
    bitset_.clear();
    return signals;
  }

  bool operator[](signal_type addr) const {
    return bitset_.find(addr) != bitset_.end();
  }

  void clearSignals() {
    bitset_.clear();
  }

  void close() {
    // Close socket
    if (sockfd>=0) {
      ::close(sockfd);
      sockfd = -1;
    }
  }

  ~UDPServer() {
    close();
  }

private:
  std::unordered_set<signal_type> bitset_;
};




#endif //GENERALFRAMEWORK_UDPSERVER_H