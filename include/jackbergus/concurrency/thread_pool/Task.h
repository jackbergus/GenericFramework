//
// Created by Giacomo Bergami, PhD on 10/04/2026.
//

#ifndef GENERALFRAMEWORK_TASK_H
#define GENERALFRAMEWORK_TASK_H

#include <cstdint>
#include <ratio>
#include <cpp/Timestamp.h>

typedef uint64_t(*task_ptr)(const char*, uint64_t);

namespace jackbergus {
    namespace concurrency {
        namespace thread_pool {
            struct Task {
                // Cannot use real numbers for defining lexicographical orderings [Debreu (1954)], and then I am forced to use integers.
                // Otherwise, I would have need to use hyperreal numbers.
                // Debreu 1954: "Representation of a Preference ordering by a numerical function". _Decision processes_, 3, 159-165
                uint64_t      computed_priority     = 0;
                uint32_t      provided_time         = 0;
                uint32_t      estimated_duration    = 0;
                uint32_t      deadline              = 0;
                task_ptr    task_to_run             = nullptr;
                const char* buffer_data             = nullptr;
                uint64_t    buffer_size             = 0;
                // Node<uint32_t>* isIntervalOfTree  = nullptr;

                Task();

                Task(const Task&) = default;
                Task(Task&&x);

                Task& operator=(const Task&) = default;
                Task& operator=(Task&& x);

                /**
                 *
                 * @tparam time_granularity Granularity for the minimal time sensibility required by the specs
                 * @param duration_tg
                 * @return The task described as being scheduled from now (even though it might take some more time)
                 */
                template<typename time_granularity = std::milli>
                static Task makeTask(uint32_t duration_tg,
                                     task_ptr task,
                                     const char* buffer_data,
                                     uint64_t buffer_size,
                                     uint32_t deadline_tg = 0) {
                    Task t;
                    t.computed_priority = 0;
                    t.provided_time = getTimestamp<time_granularity>();
                    t.estimated_duration = duration_tg;
                    t.deadline = deadline_tg ? deadline_tg : duration_tg + t.provided_time;
                    t.task_to_run = task;
                    t.buffer_data = buffer_data;
                    t.buffer_size = buffer_size;
                    return t;
                }

                friend bool operator<(const Task &lhs, const Task &rhs);
                friend bool operator<=(const Task &lhs, const Task &rhs);
                friend bool operator>(const Task &lhs, const Task &rhs);
                friend bool operator>=(const Task &lhs, const Task &rhs);
            };
        } // thread_pool
    } // concurrency
} // jackbergus

#endif //GENERALFRAMEWORK_TASK_H