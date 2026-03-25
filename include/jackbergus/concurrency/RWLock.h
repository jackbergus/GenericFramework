//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_RWLOCK_H
#define GENERALFRAMEWORK_RWLOCK_H


#include <mutex>
#include <shared_mutex>
#include <syncstream>
#include <thread>
#include <memory>

#include <jackbergus/framework/monitor/AbstractVariableMonitor.h>

namespace jackbergus::concurrency {
        template<typename Type>
        class RWLock : public framework::AbstractVariableMonitor<Type>
        {
            framework::AbstractVariableMonitor<Type> obj;
        public:
            RWLock(std::shared_ptr<framework::AbstractVariableMonitor<Type>>& obj) : obj(obj) {}

            [[nodiscard]] const FinestScaleTimeRepreentation getCurrentTime() const override {
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

            virtual bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t) override {
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
            virtual bool updateValue(FinestScaleTimeRepreentation curr_t,
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