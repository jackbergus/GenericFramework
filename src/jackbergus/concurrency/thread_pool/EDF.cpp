//
// Created by mbda on 10/04/2026.
//

#include <jackbergus/concurrency/thread_pool/EDF.h>

namespace jackbergus {
    namespace concurrency {
        namespace thread_pool {
            EDF::EDF(uint64_t jobid): jobid_(jobid) { }

            EDF::~EDF() {
            }

            void EDF::start() {
                if ((!have_all_terminated) && (!terminate_cmd) && (!isStarted)) {
                    isStarted = true;
                    threads_in_pool = std::thread([this]() {
                        while (true) {
                            Task top;
                            {
                                ThreadPoolMonitor::CS lock(std::move(hlm.lock()));
                                // std::unique_lock<std::mutex> lock{monitor};
                                if (task_queue.empty()) {
                                    if ((!have_all_terminated) && (!terminate_cmd))
                                        lock.waitCond(ThreadPoolEvents::HasSomeTaskOccurrence, -1);
                                    else
                                        break;
                                }
                                if (terminate_cmd || have_all_terminated) {
                                    break;
                                }
                                auto currentsize = task_queue.size();
                                top = task_queue.top();
                                task_queue.pop();
                                // std::cout << "#" << jobid_ << ": from " << currentsize << " to " << task_queue.size() << std::endl;
                            }
                            top.task_to_run(top.buffer_data, top.buffer_size);
                        }
                        {
                            // std::unique_lock<std::mutex> lock{monitor};
                            if (terminate_cmd) {
                                // If there are no more tasks and I am asking the threads to terminate, then I am sending the singal to all threads to terminate
                                //if (!have_all_termi nated)
                                have_all_terminated = true;
                                terminate_cmd = true;
                            }
                        }

                    });
                }
            }

            bool EDF::submitJob(const Task &task, bool forceInsertion, PriorityComputationType priority) {
                // std::unique_lock<std::mutex> lock{monitor};
                // lock.lock();
                bool returnValue = false;
                {
                    ThreadPoolMonitor::CS lock(std::move(hlm.lock()));
                    if (have_all_terminated || terminate_cmd || (!isStarted)) {
                        // If the thread is either terminated or about to terminate, then return false, as the task is not going to be submitted anyway
                        returnValue = false;
                    } else {
                        auto current_timestamp = getTimestamp<>();
                        Interval<uint32_t> i{current_timestamp, std::max(task.deadline, task.estimated_duration+current_timestamp)};
                        if (forceInsertion || (!it.lookup(i))) {
                            // Inserting the task only if the algorithm is not going to make me miss a deadline
                            if (!it.insertInterval(i)) {
                                returnValue = false;
                            } else {
                                auto t = task;
                                t.provided_time = current_timestamp;
                                // For the simple algorithm, there is no other priority than the deadline that needs to be met.
                                switch (priority) {
                                    case EDF_DeadlinePriority:
                                        t.computed_priority = t.deadline;
                                        break;
                                    case EDF_DeadlineWithActualDeadlineAndOriginalOne:
                                        t.computed_priority =  static_cast<uint64_t>(std::max(std::numeric_limits<uint32_t>::max(), t.provided_time+t.estimated_duration)) << 32;
                                        t.computed_priority += t.deadline;
                                        break;
                                }
                                task_queue.push(t);
                                returnValue = true;
                            }
                        } else {
                            returnValue = false;
                        }
                    }
                    if (returnValue)
                        lock.signalCond(ThreadPoolEvents::HasSomeTaskOccurrence, -1);
                }
                return returnValue;
            }

            bool EDF::terminate() {
                bool retVal = false;
                // std::unique_lock<std::mutex> lock{monitor};
                // lock.lock();
                {
                    ThreadPoolMonitor::CS lock(std::move(hlm.lock()));
                    if (!isStarted) {
                        retVal =  false;
                    } else {
                        if ((!have_all_terminated) && (!terminate_cmd)) {
                            while (!task_queue.empty()) {
                                // noop, polling and waiting
                            }
                            terminate_cmd = true;
                            if (task_queue.empty()) {
                                lock.signalCond(ThreadPoolEvents::HasSomeTaskOccurrence, -1);
                                lock.unlock();
                                threads_in_pool.join();
                            }
                        }
                        /*if (!have_all_terminated)
                    thread.join();*/
                        retVal =  true;
                    }
                }
                return retVal;
            }
        } // thread_pool
    } // concurrency
} // jackbergus