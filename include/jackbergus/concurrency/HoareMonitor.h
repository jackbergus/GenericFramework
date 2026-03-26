//
// Created by gyankos on 06/03/26.
//

#ifndef GENERALFRAMEWORK_HOAREMONITOR_H
#define GENERALFRAMEWORK_HOAREMONITOR_H

#include <magic_enum/magic_enum.hpp>
// #include <cpp/Semaphore.h>
#include <queue>
#include <stack>

#include "../../../submodules/vxworks_linux/include/cpp/Semaphore.h"

namespace jackbergus {
    namespace concurrency {

        template<typename CONDITIONS_ENUM>
        class HoareMonitor {
            Semaphore mutex;
            std::stack<Semaphore*> urgent;
            std::array<std::queue<Semaphore*>, magic_enum::enum_count<CONDITIONS_ENUM>()> waiting;

            public:
            HoareMonitor(): mutex(0, 1) {

            }

            bool mutex_in(int timeout) {
                return mutex.P(timeout);
            }

            bool waitCond(CONDITIONS_ENUM val, int timeout) {
                auto idx = static_cast<uint64_t>(val);
                auto ws = new Semaphore(0, 0);
                waiting[idx].push(ws);
                bool result = true;
                if (urgent.empty()) {
                    if (!mutex.V())
                        result = false;
                } else {
                    auto s = urgent.top();
                    urgent.pop();
                    if (!s->V())
                        result = false;
                }
                if (!ws->P(timeout))
                    result = false;
                delete ws;
                return result;
            }

            bool signalCond(CONDITIONS_ENUM val, int timeout) {
                auto idx = static_cast<uint64_t>(val);
                if (!waiting[idx].empty()) {
                    auto ws = waiting[idx].top();
                    auto s = new Semaphore(0, 0);
                    urgent.push(s);
                    auto result = ws->V();
                    if (!s->P(timeout))
                        result = false;
                    delete s;
                    return result;
                } else
                    return true;
            }

            bool mutex_out() {
                if (urgent.empty()) {
                    return mutex.V();
                } else {
                    auto s = urgent.top();
                    urgent.pop();
                    return s->V();
                }
            }
        };
    } // concurrency
} // jackbergus

#endif //GENERALFRAMEWORK_HOAREMONITOR_H