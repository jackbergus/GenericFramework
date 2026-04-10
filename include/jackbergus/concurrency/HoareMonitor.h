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

#include <cpp/Semaphore.h>

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
            HoareMonitor(HoareMonitor&& monitor)  noexcept = default;
            HoareMonitor& operator=(HoareMonitor&& monitor)  noexcept = default;
            HoareMonitor(const HoareMonitor&) = delete; // Cannot copy a monitor!
            HoareMonitor& operator=(const HoareMonitor&) = delete;

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
                    auto ws = waiting[idx].front();
                    waiting[idx].pop();
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

        template<typename CONDITIONS_ENUM>
        class CriticalSection;

        template<typename CONDITIONS_ENUM>
        class HighLevelHoareMonitor {
            HoareMonitor<CONDITIONS_ENUM> low_level_monitor;

        public:
            using CS = CriticalSection<CONDITIONS_ENUM>;
            HighLevelHoareMonitor() {}
            HighLevelHoareMonitor(HighLevelHoareMonitor&&) = default;
            HighLevelHoareMonitor& operator=(HighLevelHoareMonitor&&) = default;
            HighLevelHoareMonitor(const HighLevelHoareMonitor&) = delete;
            HighLevelHoareMonitor& operator=(const HighLevelHoareMonitor&) = delete;
            CriticalSection<CONDITIONS_ENUM> lock();
        };

        template<typename CONDITIONS_ENUM>
        class CriticalSection {
            bool isClosed;
            HoareMonitor<CONDITIONS_ENUM>* ref;
        public:

            CriticalSection(HoareMonitor<CONDITIONS_ENUM>* ref) : ref(ref), isClosed(false) {}
            CriticalSection(CriticalSection&&x) : ref{x.ref}, isClosed(x.isClosed) {
                x.ref = nullptr;
                x.isClosed = true;
            }
            CriticalSection& operator=(CriticalSection&& x) {
                ref = x.ref;
                isClosed = x.isClosed;
                x.ref = nullptr;
                x.isClosed = true;
                return *this;
            }
            CriticalSection(const CriticalSection&) = delete;
            CriticalSection& operator=(const CriticalSection&) = delete;
            ~CriticalSection() {
                unlock();
            }

            bool waitCond(CONDITIONS_ENUM val, int timeout) {
                if (isClosed || (!ref)) {
                    return false;
                } else {
                    return ref->waitCond(val, timeout);
                }
            }

            bool signalCond(CONDITIONS_ENUM val, int timeout) {
                if (isClosed || (!ref)) {
                    return false;
                } else {
                    return ref->signalCond(val, timeout);
                }
            }

            bool unlock() {
                if (isClosed || (!ref)) {
                    return false;
                } else {
                    ref->mutex_out();
                    isClosed = true;
                    return true;
                }
            }
        };

        template<typename CONDITIONS_ENUM>
        CriticalSection<CONDITIONS_ENUM> HighLevelHoareMonitor<CONDITIONS_ENUM>::lock() {
            return {&low_level_monitor};
        }
    } // concurrency
} // jackbergus

#endif //GENERALFRAMEWORK_HOAREMONITOR_H