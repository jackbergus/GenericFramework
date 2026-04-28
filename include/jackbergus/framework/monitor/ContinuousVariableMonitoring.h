// ContinuousVariableMonitoring.h
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

#ifndef GENERALFRAMEWORK_CONTINUOUSVARIABLEMONITORING_H
#define GENERALFRAMEWORK_CONTINUOUSVARIABLEMONITORING_H

#include <fkYAML/node.hpp>
#include <cstring>
#include <vector>
#include <jackbergus/framework/types/NativeTypes.h>
#include <memory>


#include <jackbergus/framework/ndp/FileSerializer.h>
#include <jackbergus/framework/monitor/ContinuousMonitoring.h>

namespace jackbergus {
    namespace framework {

        template <typename Type>
        class ContinuousVariableMonitoring : public  ContinuousMonitoring<Type> {
            FinestScaleTimeRepresentation start_time, end_time_inclusive;
            Type current_value;
            bool is_value_valid;

            std::vector<VariableMonitoring<Type>> variables;
            std::unique_ptr<FileSerializer<>> file_serialized;
            std::string FileName_;

            void _pushRecordNonRecursive(VariableMonitoring<Type>&& obj) {
                if (file_serialized) {
                    BlockHeader block{};
                    block.start = obj.start_time;
                    block.end = obj.end_time_inclusive;
                    block.start_validity = 1;
                    block.end_validity = 1;
                    memset(block.logger_record, 0, sizeof(block.logger_record));
                    strncpy(block.logger_record, file_serialized->getCFileName(), std::min(file_serialized->getFileNameLen()-1, sizeof(block.logger_record)));
                    block.payload_size = sizeof(Type);
                    file_serialized->write(block, (void*)&obj.value, block.payload_size);
                } else {
                    variables.emplace_back(std::move(obj));
                }
            }

            void _pushRecordNonRecursive(const VariableMonitoring<Type>& obj) {
                if (file_serialized) {
                    BlockHeader block{};
                    block.start = obj.start_time;
                    block.end = obj.end_time_inclusive;
                    block.start_validity = 1;
                    block.end_validity = 1;
                    memset(block.logger_record, 0, sizeof(block.logger_record));
                    strncpy(block.logger_record, file_serialized->getCFileName(), std::min(file_serialized->getFileNameLen()-1, sizeof(block.logger_record)));
                    block.payload_size = sizeof(Type);
                    file_serialized->write(block, (void*)&obj.value, block.payload_size);
                } else {
                    variables.emplace_back(obj);
                }
            }

            void pushRecord(const VariableMonitoring<Type>& obj) {
                if (!variables.empty()) {
                    if (file_serialized) {
                        for (const auto& record : variables) {
                            _pushRecordNonRecursive(record);
                        }
                        variables.clear();
                    }
                }
                _pushRecordNonRecursive(obj);
            }

            void pushRecord(VariableMonitoring<Type>& obj) {
                if (!variables.empty()) {
                    if (file_serialized) {
                        for (const auto& record : variables) {
                            _pushRecordNonRecursive(record);
                        }
                        variables.clear();
                    }
                }
                _pushRecordNonRecursive(std::move(obj));
            }

        public:
            ContinuousVariableMonitoring(FinestScaleTimeRepresentation start_time) : start_time(start_time), end_time_inclusive(start_time), is_value_valid(false), file_serialized{nullptr} {}
            ContinuousVariableMonitoring(FinestScaleTimeRepresentation start_time, const Type& value) : start_time(start_time), end_time_inclusive(start_time), current_value(value), is_value_valid(true), file_serialized{nullptr} {}

            ContinuousVariableMonitoring(const ContinuousVariableMonitoring<Type>& other) = default;
            ContinuousVariableMonitoring(ContinuousVariableMonitoring<Type>&& other) = default;
            ContinuousVariableMonitoring& operator=(const ContinuousVariableMonitoring<Type>& other) = default;
            ContinuousVariableMonitoring& operator=(ContinuousVariableMonitoring<Type>&& other) = default;
            ~ContinuousVariableMonitoring() {
                clearFile();
            }

            void clearFile() override {
                if (file_serialized) {
                    if (!FileName_.empty())
                    {
                        fkyaml::node node = {{"name", FileName_},  {"fields", fkyaml::node::mapping()}};
                        auto& field_struct = node["fields"].as_map();
                        auto struct_field = fkyaml::node::mapping();
                        struct_field["field_name"] = "self";
                        std::string type_name_from_enum = "_not_supported_";
                        if constexpr (std::is_integral<Type>::value) {
                            if constexpr (std::is_signed<Type>::value) {
                                type_name_from_enum = std::string( magic_enum::enum_name(type_cases::T_SIGNED_INTEGRAL));
                            } else {
                                type_name_from_enum = std::string( magic_enum::enum_name(type_cases::T_U_INTEGRAL));
                            }
                        } else if constexpr (std::is_floating_point<Type>::value) {
                            if constexpr (std::is_signed<Type>::value) {
                                type_name_from_enum = std::string( magic_enum::enum_name(type_cases::T_SIGNED_FLOAT));
                            } else {
                                type_name_from_enum = std::string( magic_enum::enum_name(type_cases::T_U_FLOAT));
                            }
                        } else if constexpr (std::is_enum<Type>::value) {
                            type_name_from_enum = std::string( magic_enum::enum_name(type_cases::T_ENUM));
                        }
                        struct_field["field_type"] = type_name_from_enum;

                        struct_field["field_type_native_size"] = sizeof(Type);
                        struct_field["binary"] = FileName_;
                        field_struct["self"] = struct_field;
                        std::ofstream f{FileName_+".yaml"};
                        f << node << std::endl;
                    }
                    file_serialized->close();
                    file_serialized = nullptr;
                }
            }

            void setFile(const std::string& FileName) override {
                clearFile();
                FileName_ = FileName;
                file_serialized = std::make_unique<FileSerializer<>>(FileName);
            }

            bool setInvalidValue(FinestScaleTimeRepresentation curr_t) override {
                if (end_time_inclusive > curr_t) {
                    return false;
                }
                if (!is_value_valid) {
                    return true;
                } else {
                    VariableMonitoring<Type> s;
                    s.start_time = start_time;
                    s.end_time_inclusive = end_time_inclusive;
                    s.value = current_value;
                    pushRecord(std::move(s));
                    start_time = end_time_inclusive = curr_t;
                    is_value_valid = false;
                    return true;
                }
            }

            [[nodiscard]] const FinestScaleTimeRepresentation getCurrentTime() const  override {
                return end_time_inclusive;
            };
            [[nodiscard]] bool isCurrentlyValid() const override {
                return is_value_valid;
            };
            [[nodiscard]] void* getRawPtr() const override {
                return is_value_valid ? (void*)&current_value : nullptr;
            };

            bool updateValue(FinestScaleTimeRepresentation curr_t,
                             const Type& value) override {
                return updateValue(curr_t, value, {});
            }

            /**
             *
             * @param curr_t Time when the value was recorded
             * @param value Value being recorded at the current time
             * @param t Optional equality predicate. If missing, it is using the default one
             * @return Whether the value was successfully updated
             */
            bool updateValue(FinestScaleTimeRepresentation curr_t,
                             const Type& value,
                             const std::equal_to<Type>& t) {
                if (end_time_inclusive > curr_t) {
                    return false;
                }
                if (!is_value_valid) {
                    start_time = end_time_inclusive = curr_t;
                    current_value = value;
                    is_value_valid = true;
                } else {
                    if (t(current_value, value)) {
                        end_time_inclusive = curr_t;
                    } else {
                        VariableMonitoring<Type> s;
                        s.start_time = start_time;
                        s.end_time_inclusive = end_time_inclusive;
                        s.value = current_value;
                        pushRecord(std::move(s));
                        start_time = end_time_inclusive = curr_t;
                        current_value = value;
                        is_value_valid = true;
                    }
                }
                return true;
            }

            bool flush() {
                if (file_serialized) {
                    file_serialized->flush();
                    return true;
                } else {
                    return false;
                }
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_CONTINUOUSVARIABLEMONITORING_H
