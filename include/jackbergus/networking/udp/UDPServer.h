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
#include <vector>
#include <unordered_set>

template
<typename signal_type>
class UDPServer {
  std::string server_ip;
  int         server_port;
#ifndef USE_ZMQ
   struct sockaddr_in servaddr, cliaddr;
#endif
  int         rc;
  void *ctx{nullptr};
  void *dish{nullptr};
  bool doWait;
  std::string mcast_group;

  UDPServer(const std::string& ip, int port, bool doWait, unsigned long sec_wait = 0, unsigned long usec_wait = 10) : server_ip(ip), server_port(port), doWait(doWait) {
#ifdef USE_ZMQ
    ctx = zmq_ctx_new();
    dish = zmq_socket(ctx, ZMQ_DISH);
    mcast_group = "udp://" + server_ip + ":" + std::to_string(server_port);
    rc = zmq_bind(dish, mcast_group.c_str());
    zmq_join(dish, MCAST_GROUP);
#else
    struct timeval read_timeout;
    rc = socket(AF_INET, SOCK_DGRAM, 0);
    if (!doWait) {
      read_timeout.tv_sec = sec_wait;
      read_timeout.tv_usec = usec_wait;
    }
    if (rc > 0) {
      int OptVal = 1;
      auto ris = setsockopt(rc, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
      if (ris == -1)  {
        printf ("setsockopt() SO_REUSEADDR failed, Errno: %d \"%s\"  SOCKET_ERROR=%d\n",
            errno,strerror(errno), GetWSASocketError(rc));
        rc = -2;
      } else {
        if (!doWait) {
#if defined(MINGW_DDK_H) || defined(WIN32) || defined(WIN64) || defined(__MINGW64__) || defined(__MINGW32__)
          //The SO_RCVTIMEO socket option in Windows sets a timeout (in milliseconds) for blocking receive calls like recv().
          DWORD timeout = read_timeout.tv_sec *1000 + read_timeout.tv_usec/1000000;
          timeout = std::max(timeout, (DWORD)10);
          ris = setsockopt(rc, SOL_SOCKET, SO_RCVTIMEO,(char *) &timeout, sizeof timeout);
#else
          ris = setsockopt(rc, SOL_SOCKET, SO_RCVTIMEO,(char *) &read_timeout, sizeof read_timeout);
#endif
          if (ris == -1)  {
            printf ("setsockopt() SO_RCVTIMEO failed, Errno: %d \"%s\"  SOCKET_ERROR=%d\n",
                errno,strerror(errno), GetWSASocketError(rc));
            rc = -3;
          }
        }
        if (rc >= 0) {
          struct sockaddr_in Local;
          memset(&Local, 0, sizeof(servaddr));
          // Fill server address info
          Local.sin_family = AF_INET;              // IPv4
          Local.sin_port   = htons(port);          // Server port
          Local.sin_addr.s_addr = inet_addr(ip.c_str()); // Server IP
          ris = bind(rc, (struct sockaddr*) &Local, sizeof(Local));
          if (ris == -1)  {
            printf ("listen() failed, Err: %d \"%s\"  SOCKET_ERROR=%d\n",
                errno, strerror(errno), GetWSASocketError(rc));
            rc = -4;
          }
        }
      }

    }
#endif
  }

public:
  // static constexpr uint64_t MAX_VAL = (1 << magic_enum::detail::range_max<signal_type>::value);

  static UDPServer* instance(const std::string& ip, int port, bool doWait = false, unsigned long sec_wait = 0, unsigned long usec_wait = 10) {
    if (!InitNetworking::getInstance()->good())
      return nullptr;
    return new UDPServer(ip, port, doWait, sec_wait, usec_wait);
  }

  bool recv_signal(void) {
#ifdef USE_ZMQ
    zmq_msg_t recv_msg;
    zmq_msg_init (&recv_msg);
    auto flag = doWait ? 0 : ZMQ_DONTWAIT;
    signal_type result;
    int val = zmq_recvmsg (dish, &recv_msg, flag );
    bool resultIsGood;
    if (val == sizeof(result)) {
      result = *(signal_type*)zmq_msg_data(&recv_msg);
      bitset_.insert(result);
      resultIsGood = true;
    } else {
      std::cout << strerror(errno) << std::endl;
      resultIsGood = false;
    }
    zmq_msg_close (&recv_msg);
    return resultIsGood;
#else
    if (rc < 0) {
      return false;
    }
    socklen_t len;
    int n;
    len = sizeof(cliaddr);  //len is value/result
    signal_type result;
    n = recvfrom(rc, (char *)&result, sizeof(signal_type),
                0, ( struct sockaddr *) &cliaddr,
                &len);
    if (n==sizeof(result)) {
      bitset_.insert(result);
      return true;
    } else {
      // std::cout << strerror(errno) << std::endl;
      return false;
    }
#endif
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
#ifdef USE_ZMQ
    if (dish) {
      zmq_close(dish);
      dish = nullptr;
    }
    if (ctx) {
      zmq_ctx_term(ctx);
      ctx = nullptr;
    }
#else
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

  ~UDPServer() {
    close();
  }

private:
  std::unordered_set<signal_type> bitset_;
};




#endif //GENERALFRAMEWORK_UDPSERVER_H
