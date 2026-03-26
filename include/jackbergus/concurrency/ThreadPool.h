//
// Created by gyankos on 06/03/26.
//

#ifndef GENERALFRAMEWORK_THREADPOOL_H
#define GENERALFRAMEWORK_THREADPOOL_H
#include <cstdint>

namespace jackbergus {
    namespace concurrency {

        template <uint64_t N_threads = 4>
        class ThreadPool {
        public:
            class ThreadCtrlBlk
            {
            public:
                int id;
                long period;
                long task_time;
                long deadline;
            };


            enum scheduling_type {
                FIFO_SCHEDULE = 1,
                EDF_SCHEDULE = 2,
                RM_SCHEDULE = 3
            };
        };
    } // concurrency
} // jackbergus

#endif //GENERALFRAMEWORK_THREADPOOL_H