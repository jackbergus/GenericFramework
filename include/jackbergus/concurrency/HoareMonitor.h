// HoareMonitor.h
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