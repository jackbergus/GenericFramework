//
// Created by mbda on 10/04/2026.
//

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