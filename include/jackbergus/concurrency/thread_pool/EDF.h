// EDF.h
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

#ifndef GENERALFRAMEWORK_EDF_H
#define GENERALFRAMEWORK_EDF_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue> // also providing the priority queue

#include <jackbergus/data_structures/IntervalTree.h>
#include <jackbergus/concurrency/thread_pool/Task.h>
#include <jackbergus/concurrency/HoareMonitor.h>

enum class ThreadPoolEvents;

namespace jackbergus {
    namespace concurrency {
        namespace thread_pool {


enum PriorityComputationType {
    EDF_DeadlinePriority = 0,
    EDF_DeadlineWithActualDeadlineAndOriginalOne = 1,
};

            enum class ThreadPoolEvents {
                HasSomeTaskOccurrence
            };

struct EDF  {
    explicit EDF(uint64_t jobid = 0);

    ~EDF();

    void start();

    bool submitJob(const Task& task,
                   bool forceInsertion = false,
                   PriorityComputationType priority = EDF_DeadlinePriority);

    bool terminate();

    uint64_t jobid_;

    using ThreadPoolMonitor = HighLevelHoareMonitor<ThreadPoolEvents>;
private:
    IntervalTree<uint32_t> it;
    bool isStarted = false;
    bool terminate_cmd = false;
    bool have_all_terminated = false;
    std::thread threads_in_pool;
    // std::condition_variable wake_up_waiting_thread;
    std::priority_queue<Task> task_queue;
    ThreadPoolMonitor hlm;
    //std::mutex monitor;
    //std::condition_variable has_some_tasks;
    // std::thread thread;
};


        } // thread_pool
    } // concurrency
} // jackbergus

#endif //GENERALFRAMEWORK_EDF_H