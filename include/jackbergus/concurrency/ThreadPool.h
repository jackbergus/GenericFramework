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