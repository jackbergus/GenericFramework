//
// Created by Giacomo Bergami on 05/05/2026.
//

#include <jackbergus/networking/InitNetworking.h>
#if defined(MINGW_DDK_H) || defined(WIN32) || defined(WIN64) || defined(__MINGW64__) || defined(__MINGW32__)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

std::unique_ptr<InitNetworking> InitNetworking::self{nullptr};

InitNetworking::InitNetworking() {
#if defined(MINGW_DDK_H) || defined(WIN32) || defined(WIN64) || defined(__MINGW64__) || defined(__MINGW32__)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        good_ = false;
    } else {
        good_ = true;
    }
#else
    good_ = true;
#endif
}

void InitNetworking::close() {
#if defined(MINGW_DDK_H) || defined(WIN32) || defined(WIN64) || defined(__MINGW64__) || defined(__MINGW32__)
    WSACleanup();
#endif
}
