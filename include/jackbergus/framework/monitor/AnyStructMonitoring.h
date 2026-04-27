// AnyStructMonitoring.h
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

#ifndef GENERALFRAMEWORK_ANYSTRUCTMONITORING_H
#define GENERALFRAMEWORK_ANYSTRUCTMONITORING_H

#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <vector>

//#include <field_reflection.hpp> Removed. as it was not compatible with C++17 TODO: conditional compiling depending on the version of C++
#include "refl.hpp"

#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h>

#include "jackbergus/data/recursive_encoder.h"

#include <fkYAML/node.hpp>

namespace jackbergus {
    namespace framework {

        template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_for {
            std::vector<AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepresentation start_time, const std::string& base_path = "") {
                std::vector<AnyFundamentalVariableMonitoring<block_size>> current;
                auto view = std::string(refl::trait::get_t<x, refl::member_list<T>>::name.c_str());
                using K = typename refl::trait::get_t<x, refl::member_list<T>>::value_type;
                constexpr auto val = getTypeInformation<K>();
                if constexpr (val == type_cases::T_CLASS) {
                    // if I am now dealing with a class, then consider recursively wrapping this into something else,
                    // and re-start the computation back again
                    current = static_for<K, 0, refl::member_list<K>::size>{}.expandWithBasicMonitor(start_time, base_path + view + ".");
                } else if constexpr (val == type_cases::T_STATIC_ARRAY) {
                   // If this is an array, then I need to consider whether the internal element is a fundamental type or not
                   using H = typename std::remove_all_extents_t<K>;
                   constexpr uint64_t N = sizeof(K)/sizeof(H);
                   if constexpr (!std::is_fundamental_v<H>) {
                       for (auto i = 0u; i < N; ++i) {
                           auto local = static_for<H, 0, refl::member_list<H>::size>{}.expandWithBasicMonitor(start_time, base_path+view+"["+std::to_string(i)+"].");
                           current.insert(current.end(), std::move_iterator(local.begin()), std::move_iterator(local.end()));
                       }
                   } else {
                       for (auto i = 0u; i < N; ++i) {
                           current.emplace_back(flatten_type_to_enum<H>(start_time, x, base_path+view+"["+std::to_string(i)+"]"));
                       }
                   }
                } else {
                    // Otherwise, just return the element as it stands, forsooth!
                    current.emplace_back(flatten_type_to_enum<K>(start_time, x, base_path+view));
                }
                auto result = static_for<T, x+1,to>().expandWithBasicMonitor(start_time, base_path);
                current.insert(current.end(), std::move_iterator(result.begin()), std::move_iterator(result.end()));
                return current;
            }

            uint64_t setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepresentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f, uint64_t acc = 0) {
                auto val = get_field_t<T, x>::get(value);
                using K = typename refl::trait::get_t<x, refl::member_list<T>>::value_type;
                constexpr auto t_val = getTypeInformation<K>();
                if constexpr (t_val == type_cases::T_STATIC_ARRAY) {
                    using H = typename std::remove_all_extents_t<K>;
                    constexpr uint64_t N = sizeof(K)/sizeof(H);
                    if constexpr (!std::is_fundamental_v<H>) {
                        for (auto i = 0u; i<N; i++) {
                            acc = static_for<H, 0, refl::member_list<H>::size>{}.setRecursivelyWithTemplates(start_time, val[i], f, acc);
                        }
                        return static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f, acc);
                    } else {
                        for (auto i = 0u; i < N; ++i) {
                            f[acc].updateValue(start_time, val[i]);
                            acc++;
                        }
                        return static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f, acc);
                    }
                } else if constexpr (t_val != type_cases::T_CLASS) {
                    // if this is merely a field, then doing the immediate update
                    f[acc].updateValue(start_time, val);
                    return static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f, acc+1);
                } else {
                    // otherwise, I need to recursively analyse this by not updating the field directly, rather one of
                    // its constituents
                    auto withInDepthRecursion = static_for<K, 0, refl::member_list<K>::size>{}.setRecursivelyWithTemplates(start_time, val, f, acc);
                    return static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f, withInDepthRecursion);
                }
            }
        };

        template<typename T, int to, uint64_t block_size>
        struct static_for<T, to,to, block_size> {
            std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepresentation start_time, const std::string& base_path = "") {
                return {};
            }

            uint64_t setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepresentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f, uint64_t acc = 0) {
                return acc;
            }
        };

        template <typename  T, uint64_t block_size=1024> std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> getNativeType(jackbergus::framework::FinestScaleTimeRepresentation start_time) {
            auto v = static_for<T, 0, refl::member_list<T>::size>{}.expandWithBasicMonitor(start_time, "");
            //std::reverse(v.begin(), v.end());
            return v;
        }

        /**
         * Serializes and persists a record by serializing each field separately
         * @tparam T              Struct of native types to serialize
         * @tparam block_size
         */
        template <typename  T, uint64_t block_size = 1024>
                class AnyStructMonitoring : public jackbergus::framework::ContinuousMonitoring<T> {

            std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> fields;
            std::string FileName_;
        public:
            constexpr static uint64_t field_count = refl::member_list<T>::size; //field_reflection::field_count<T>;
            using validity_vector = std::array<bool, field_count>;

            [[nodiscard]] const FinestScaleTimeRepresentation getCurrentTime() const  override {
                FinestScaleTimeRepresentation t = 0;
                for (const auto& ref : fields) {
                    if (ref.validityInterval().second > t) {
                        t = ref.validityInterval().second;
                    }
                }
                return t;
            };
            [[nodiscard]] bool isCurrentlyValid() const override {
                for (const auto& ref : fields) {
                    if (ref.isCurrentlyValid()) {
                        return true;
                    }
                }
                return false;
            }
            [[nodiscard]] virtual void* getRawPtr() const override {
                throw std::invalid_argument("getRawPtr()");
            }

            bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t) override {
                auto result = true;
                for (auto& ref : fields) {
                    if (!ref.setInvalidValue(curr_t))
                        result = false;
                }
                return result;
            }

            bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                             const T& value)  override {
                static_for<T, 0, refl::member_list<T>::size>{}.setRecursivelyWithTemplates(curr_t, value, fields, 0);
                return true;
            }

            AnyStructMonitoring(jackbergus::framework::FinestScaleTimeRepresentation start_time) {
                fields = getNativeType<T>(start_time);
                setInvalidValue(start_time);
            }

            AnyStructMonitoring(jackbergus::framework::FinestScaleTimeRepresentation start_time, const T& value) {
                fields = getNativeType<T>(start_time);
                updateValue(start_time, value);
            }

            virtual ~AnyStructMonitoring() {
                clearFile();
            }

            void clearFile() override {
                fkyaml::node node = {{"name", FileName_},  {"fields", fkyaml::node::mapping()}};
                auto& field_struct = node["fields"].as_map();
                for (auto& ref : fields) {
                    std::string val = ref.field_name();
                    auto struct_field = fkyaml::node::mapping();
                    struct_field["field_name"] = val;
                    struct_field["field_type"] = std::string( magic_enum::enum_name(ref.field_type()));
                    struct_field["field_type_native_size"] = ref.native_size();
                    struct_field["binary"] = FileName_+"_"+ref.field_name()+".bin";
                    field_struct[val] = struct_field;
                    ref.clearFile();
                }
                if (!FileName_.empty())
                {
                    std::ofstream f{FileName_+".yaml"};
                    f << node << std::endl;
                }
            }

            void setFile(const std::string& FileName) override {
                clearFile();
                FileName_ = FileName;
                for (auto& field : fields) {
                    field.setFile(FileName+"_"+field.field_name()+".bin");
                }
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_ANYSTRUCTMONITORING_H