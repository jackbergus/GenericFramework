// ContinuousMonitoring.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - gyankos
//
// narcissus is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  narcissus is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with narcissus. If not, see <http://www.gnu.org/licenses/>.

#ifndef GENERALFRAMEWORK_CONTINUOUSMONITORING_H
#define GENERALFRAMEWORK_CONTINUOUSMONITORING_H

#include <jackbergus/framework/types/NativeTypes.h>
#include <narcissus/lightweight_any.h>

#include <jackbergus/framework/monitor/AbstractVariableMonitor.h>


    namespace jackbergus::framework {
        template<typename Type>
        struct VariableMonitoring {
            FinestScaleTimeRepreentation start_time;
            FinestScaleTimeRepreentation end_time_inclusive;
            Type value;

            VariableMonitoring(FinestScaleTimeRepreentation st = 0, FinestScaleTimeRepreentation et = 0, Type v = {}) : start_time(st), end_time_inclusive(et), value(v) {}
            VariableMonitoring(const VariableMonitoring& x) : start_time(x.start_time), end_time_inclusive(x.end_time_inclusive), value(x.value) {}
            VariableMonitoring(VariableMonitoring&& x)  : start_time(x.start_time), end_time_inclusive(x.end_time_inclusive), value(x.value) {}
            VariableMonitoring& operator=(const VariableMonitoring& x) {
start_time = x.start_time;
            end_time_inclusive = x.end_time_inclusive;
            value = x.value;
            return *this;}
            VariableMonitoring& operator=(VariableMonitoring&& x) {
                start_time = x.start_time;
                end_time_inclusive = x.end_time_inclusive;
                value = x.value;
                return *this;}
        };

        using AnyVariableMonitoring = jackbergus::framework::VariableMonitoring<lightweight_any>;

        template <typename T>
        class ContinuousMonitoring : public AbstractVariableMonitor<T> {
        public:
            virtual void clearFile() = 0;
            virtual void setFile(const std::string& FileName) = 0;
            virtual bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t) = 0;
        };
    }


#endif //GENERALFRAMEWORK_CONTINUOUSMONITORING_H
