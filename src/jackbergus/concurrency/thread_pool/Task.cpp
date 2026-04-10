//
// Created by Giacomo Bergami, PhD on 10/04/2026.
//

#include "jackbergus/concurrency/thread_pool/Task.h"

namespace jackbergus {
    namespace concurrency {
        namespace thread_pool {
            Task::Task() {}

            Task::Task(Task &&x): computed_priority{x.computed_priority}, provided_time(x.provided_time), estimated_duration(x.estimated_duration), deadline(x.deadline), task_to_run(x.task_to_run), buffer_data(x.buffer_data), buffer_size(x.buffer_size) {

            }

            Task & Task::operator=(Task &&x) {
                computed_priority = x.computed_priority;
                provided_time = x.provided_time;
                estimated_duration = x.estimated_duration;
                deadline = x.deadline;
                task_to_run = x.task_to_run;
                buffer_data = x.buffer_data;
                buffer_size = x.buffer_size;
                return *this;
            }

            bool operator<(const Task &lhs, const Task &rhs) {
                return lhs.computed_priority < rhs.computed_priority;
            }

            bool operator<=(const Task &lhs, const Task &rhs) {
                return !(rhs < lhs);
            }

            bool operator>(const Task &lhs, const Task &rhs) {
                return rhs < lhs;
            }

            bool operator>=(const Task &lhs, const Task &rhs) {
                return !(lhs < rhs);
            }
        } // thread_pool
    } // concurrency
} // jackbergus