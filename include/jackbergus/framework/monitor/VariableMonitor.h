//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_VARIABLEMONITOR_H
#define GENERALFRAMEWORK_VARIABLEMONITOR_H

#include <optional>
#include <jackbergus/framework/types/NativeTypes.h>

#include "AbstractVariableMonitor.h"

namespace jackbergus {
    namespace framework {
        template<typename Type>
        class VariableMonitor : public AbstractVariableMonitor<Type> {
            FinestScaleTimeRepresentation t;
            Type val;
            bool validity;

        public:
            VariableMonitor(FinestScaleTimeRepresentation t = 0) : t(t), validity(false), val{} {}
            VariableMonitor(FinestScaleTimeRepresentation t, const Type& val) : t(t), validity(true), val{val} {}

            const FinestScaleTimeRepresentation getCurrentTime() const override {
                return t;
            }

            bool isCurrentlyValid() const override {
                return validity;
            }

            void* getRawPtr() const override {
                return validity ? &val : nullptr;
            }

            bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t) override {
                validity = false;
                if (curr_t < t) {
                    return false;
                }
                t = curr_t;
                return true;
            }

            bool updateValue(FinestScaleTimeRepresentation curr_t,
                             const Type& value) override {
                if (curr_t < t) {
                    validity = false;
                    return false;
                }
                t = curr_t;
                val = value;
                validity = true;
                return true;
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_VARIABLEMONITOR_H