// ThreadPool.h
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

#ifndef GENERALFRAMEWORK_THREADPOOL_H
#define GENERALFRAMEWORK_THREADPOOL_H
#include <cstdint>


#include <jackbergus/concurrency/thread_pool/Task.h>
#include <jackbergus/concurrency/thread_pool/EDF.h>

namespace jackbergus {
    namespace concurrency {
        namespace thread_pool {

            template<uint64_t N, uint64_t LateTaskThreads>
struct ThreadPool {
    static_assert(LateTaskThreads<N);
    static constexpr uint64_t normal_threads = N-LateTaskThreads;
    EDF                         t[N];
    // std::atomic<bool> thread_started[N];
    // std::atomic<bool>       has_work[N];
    // std::atomic<bool>           exit[N];

    ThreadPool() {
        for (uint64_t i = 0; i < N; i++) {
            t[i].jobid_ = i;
        }
    }

    void start() {
        for (uint64_t i = 0; i < N; i++) {
            t[i].start();
        }
    }

    bool submitJob(uint64_t idx, const Task& task) {
        bool result;
        if (idx >= normal_threads) {
            result = false;
        } else {
            if (! t[idx].submitJob(task, false, EDF_DeadlinePriority)) {
                result = forceComputationThatWillBeDelayedNevertheless(task);
            } else {
                result = true;
            }
        }
        return result;
    }

    bool submitJob(const Task& task) {
        uint64_t orig_value = round_robin_normies;
        bool result = false;
        for (uint64_t i = 0; i < normal_threads; i++) {
            auto curr_idx = (i + round_robin_normies) % normal_threads;
            if (t[orig_value].submitJob(task, false, EDF_DeadlinePriority)) {
                round_robin_normies = (curr_idx + 1) % normal_threads;
                result = true;
                break;
            }
        }
        if (!result) {
            forceComputationThatWillBeDelayedNevertheless(task);
        } else {
            // noop: already computed
        }
        return result;
    }

    void terminate() {
        for (uint64_t i = 0; i < N; i++) {
            t[i].terminate();
        }
    }

private:

    bool forceComputationThatWillBeDelayedNevertheless(const Task& task) {
        uint64_t orig_value = round_robin_lates;
        bool result = false;
        for (uint64_t i = 0; i < LateTaskThreads; i++) {
            auto curr_idx = (i + round_robin_lates) % LateTaskThreads;
            if (t[orig_value+normal_threads].submitJob(task, false, EDF_DeadlineWithActualDeadlineAndOriginalOne)) {
                round_robin_lates = (curr_idx + 1) % LateTaskThreads;
                result = true;
                break;
            }
        }
        if (!result) {
            round_robin_lates = (round_robin_lates+1) % LateTaskThreads;
            //None of the available threads was ready. Thus, using the round robin to determine the current
            //thread that will be forced to digest the task
            result = t[orig_value+normal_threads].submitJob(task, true, EDF_DeadlineWithActualDeadlineAndOriginalOne);
        } else {
            // noop: already computed
        }
        return result;
    }

    uint64_t round_robin_lates = 0;
    uint64_t round_robin_normies = 0;
};

        }
    } // concurrency
} // jackbergus

#endif //GENERALFRAMEWORK_THREADPOOL_H