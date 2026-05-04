//
// Created by gyankos on 26/03/26.
//

#include <cstdint>

#include <jackbergus/networking/udp/udp.h>

#include "jackbergus/networking/udp/UDPClient.h"
#include "jackbergus/networking/udp/UDPServer.h"

enum class signal_test : uint64_t {
  SIGNAL_A = 0,
  SIGNAL_B = 1,
  SIGNAL_C = 2,
};


int main(void) {
  UDPServer<signal_test>* st_server = UDPServer<signal_test>::instance("127.0.0.1", 8089);
  if (st_server == nullptr) {
    return 1;
  }
  UDPClient<signal_test>* st_client = UDPClient<signal_test>::instance("127.0.0.1", 8089);
  if (st_client == nullptr) {
    return 2;
  }
  st_client->send_signal(signal_test::SIGNAL_A);
  st_client->send_signal(signal_test::SIGNAL_C);
  while (st_server->recv_signal());
  std::cout << (*st_server)[signal_test::SIGNAL_A] << std::endl;
  std::cout << (*st_server)[signal_test::SIGNAL_B] << std::endl;
  std::cout << (*st_server)[signal_test::SIGNAL_C] << std::endl;
  auto val= st_server->getActiveSignals();
  std::cout << val.size() << std::endl;
}