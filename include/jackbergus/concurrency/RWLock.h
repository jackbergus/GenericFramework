// RWLock.h
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

#ifndef GENERALFRAMEWORK_RWLOCK_H
#define GENERALFRAMEWORK_RWLOCK_H


#include <mutex>
#include <shared_mutex>
#include <syncstream>
#include <thread>
#include <memory>

#include <jackbergus/framework/types/NativeTypes.h>
#include <jackbergus/framework/monitor/AbstractVariableMonitor.h>

namespace jackbergus::concurrency {
        template<typename Type>
        class RWLock : public framework::AbstractVariableMonitor<Type>
        {
            framework::AbstractVariableMonitor<Type> obj;
        public:
            RWLock(std::shared_ptr<framework::AbstractVariableMonitor<Type>>& obj) : obj(obj) {}

            [[nodiscard]] const jackbergus::framework::FinestScaleTimeRepresentation getCurrentTime() const override {
                if (!obj)
                    return -1;
                {
#ifndef TARGET_VXWORKS
                    std::shared_lock lock(mutex_);
#else
                    // TODO
#endif
                    return obj.getCurrentTime();
                }
            }

            [[nodiscard]] bool isCurrentlyValid() const override {
                if (!obj)
                    return false;
                {
#ifndef TARGET_VXWORKS
                    std::shared_lock lock(mutex_);
#else
                    // TODO
#endif
                    return obj.isCurrentlyValid();
                }
            }

            [[nodiscard]] void* getRawPtr() const override {
                if (!obj)
                    return nullptr;
                {
#ifndef TARGET_VXWORKS
                    std::shared_lock lock(mutex_);
#else
                    // TODO
#endif
                    return obj.getRawPtr();
                }
            }

            virtual bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t) override {
                if (!obj)
                    return false;
                {
#ifndef TARGET_VXWORKS
                    std::unique_lock lock(mutex_);
#else
                    // TODO
#endif
                    return obj.setInvalidValue(curr_t);
                }
            }
            virtual bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                             const Type& value) {
            if (!obj)
                return false;
            {
#ifndef TARGET_VXWORKS
                std::unique_lock lock(mutex_);
#else
                // TODO
#endif
                return obj.updateValue(curr_t, value);
            }
                             }

        private:
#ifndef TARGET_VXWORKS
            mutable std::shared_mutex mutex_;
#else
            // TODO
#endif
        };
    } // concurrency
// jackbergus

#endif //GENERALFRAMEWORK_RWLOCK_H