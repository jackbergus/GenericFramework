// VariableMonitor.h
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

#ifndef GENERALFRAMEWORK_VARIABLEMONITOR_H
#define GENERALFRAMEWORK_VARIABLEMONITOR_H

#include <optional>
#include <jackbergus/framework/types/NativeTypes.h>
#include <jackbergus/framework/monitor/AbstractVariableMonitor.h>

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