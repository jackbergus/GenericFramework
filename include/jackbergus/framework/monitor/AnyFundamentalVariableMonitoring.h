//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_ANYFUNDAMENTALVARIABLEMONITORING_H
#define GENERALFRAMEWORK_ANYFUNDAMENTALVARIABLEMONITORING_H

#include <cstdint>
#include <jackbergus/framework/monitor/ContinuousMonitoring.h>
#include <jackbergus/framework/monitor/NativeTypeMonitoring.h>
#include <jackbergus/framework/ndp/FileSerializer.h>
#include <jackbergus/framework/types/NativeTypes.h>
#include <cstring>
#include <vector>

namespace jackbergus {
    namespace framework {
template <uint64_t block_size = 1024>
        class AnyFundamentalVariableMonitoring : public jackbergus::framework::ContinuousMonitoring<lightweight_any> {
            jackbergus::framework::FinestScaleTimeRepreentation start_time, end_time_inclusive;
            lightweight_any current_value;
            bool is_value_valid;
            NativeTypeMonitoring type_info;

            std::vector<AnyVariableMonitoring> variables;
            std::unique_ptr<jackbergus::framework::FileSerializer<block_size>> file_serialized;

            inline void _pushRecordNonRecursive(AnyVariableMonitoring&& obj) {
                if (file_serialized) {
                    jackbergus::framework::BlockHeader block{};
                    block.start = obj.start_time;
                    block.end = obj.end_time_inclusive;
                    block.start_validity = 1;
                    block.end_validity = 1;
                    memset(block.logger_record, 0, sizeof(block.logger_record));
                    strncpy(block.logger_record, file_serialized->getCFileName(), std::min(file_serialized->getFileNameLen()-1, sizeof(block.logger_record)));
                    block.payload_size = type_info.sizeof_;
                    file_serialized->write(block, obj.value.raw(), block.payload_size);
                } else {
                    variables.emplace_back(std::move(obj));
                }
            }

            inline void _pushRecordNonRecursive(const AnyVariableMonitoring& obj) {
                if (file_serialized) {
                     jackbergus::framework::BlockHeader block{};
                    block.start = obj.start_time;
                    block.end = obj.end_time_inclusive;
                    block.start_validity = 1;
                    block.end_validity = 1;
                    memset(block.logger_record, 0, sizeof(block.logger_record));
                    strncpy(block.logger_record, file_serialized->getCFileName(), std::min(file_serialized->getFileNameLen()-1, sizeof(block.logger_record)));
                    block.payload_size = type_info.sizeof_;
                    file_serialized->write(block, obj.value.raw(), block.payload_size);
                } else {
                    variables.emplace_back(obj);
                }
            }

            inline void pushRecord(const AnyVariableMonitoring& obj) {
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

            inline void pushRecord(AnyVariableMonitoring& obj) {
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
            AnyFundamentalVariableMonitoring(jackbergus::framework::FinestScaleTimeRepreentation start_time,  NativeTypeMonitoring&& type) : start_time(start_time), type_info{std::move(type)},  end_time_inclusive(start_time), is_value_valid(false), file_serialized{nullptr} {}
            AnyFundamentalVariableMonitoring(jackbergus::framework::FinestScaleTimeRepreentation start_time,  NativeTypeMonitoring&& type, const lightweight_any& value) : type_info{std::move(type)},  start_time(start_time), end_time_inclusive(start_time), current_value(value), is_value_valid(true), file_serialized{nullptr} {}

            AnyFundamentalVariableMonitoring(const AnyFundamentalVariableMonitoring& other) = default;
            AnyFundamentalVariableMonitoring(AnyFundamentalVariableMonitoring&& other) = default;
            AnyFundamentalVariableMonitoring& operator=(const AnyFundamentalVariableMonitoring& other) = default;
            AnyFundamentalVariableMonitoring& operator=(AnyFundamentalVariableMonitoring&& other) = default;

            virtual ~AnyFundamentalVariableMonitoring() {

            }

            const std::string& field_name() const {
                return type_info.name;
            }

            const type_cases field_type() const {
                return type_info.casusu;
            }

            const uint64_t native_size() const {
                return type_info.sizeof_;
            }

            const std::type_index& type() const {
                return type_info.type_i;
            }

            void clearFile() override {
                if (file_serialized) {
                    for (const auto& record : variables) {
                        _pushRecordNonRecursive(record);
                    }
                    variables.clear();
                    AnyVariableMonitoring s;
                    s.start_time = start_time;
                    s.end_time_inclusive = end_time_inclusive;
                    s.value = current_value;
                    pushRecord(std::move(s));
                    start_time = end_time_inclusive = 0;
                    is_value_valid = false;
                    file_serialized->close();
                    file_serialized = nullptr;
                }
            }

            void setFile(const std::string& FileName) override {
                clearFile();
                file_serialized = std::make_unique<jackbergus::framework::FileSerializer<>>(FileName);
            }

            bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t) override {
                if (end_time_inclusive > curr_t) {
                    return false;
                }
                if (!is_value_valid) {
                    return true;
                } else {
                    AnyVariableMonitoring s;
                    s.start_time = start_time;
                    s.end_time_inclusive = end_time_inclusive;
                    s.value = current_value;
                    pushRecord(std::move(s));
                    start_time = end_time_inclusive = curr_t;
                    is_value_valid = false;
                    return true;
                }
            }

            bool isCurrentlyValid() const override {
                return is_value_valid;
            }

            std::pair<jackbergus::framework::FinestScaleTimeRepreentation, jackbergus::framework::FinestScaleTimeRepreentation> validityInterval() const {
                if (is_value_valid) {
                    return {start_time, end_time_inclusive};
                } else {
                    return {-1,-1};
                }
            }



            /**
             *
             * @tparam T
             * @return
             */
            template <typename T> T* get() const {
                return is_value_valid ? (T*)current_value.raw() : nullptr;
            }

    [[nodiscard]] virtual const FinestScaleTimeRepreentation getCurrentTime() const {
                return end_time_inclusive;
            }

    [[nodiscard]] virtual void* getRawPtr() const override {
        return is_value_valid ? (void*)current_value.raw() : nullptr;
    }

    bool updateValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t,
                             const lightweight_any& value) override {
                return updateValue(curr_t, value, {});
            }

            /**
             *
             * @param curr_t Time when the value was recorded
             * @param value Value being recorded at the current time
             * @param t Optional equality predicate. If missing, it is using the default one
             * @return Whether the value was successfully updated
             */
            bool updateValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t,
                             const lightweight_any& value,
                             const std::equal_to<lightweight_any>& t) {
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
                        AnyVariableMonitoring s;
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
        };
    } // framework
} // jackbergus

#include <magic_enum/magic_enum.hpp>

template <typename T, uint64_t block_size = 1024>
jackbergus::framework::AnyFundamentalVariableMonitoring<block_size> flatten_type_to_enum(jackbergus::framework::FinestScaleTimeRepreentation start_time, uint64_t val, const std::string& name) {
    std::type_index type_i = std::type_index(typeid(T));
    // bool cond = (name == "bounded_array");
    if constexpr (std::is_same_v<T, std::string>) {
        static_assert(false, "std::string not supported");
    }
    if constexpr (std::is_void_v<T>) {
        static_assert(false, "void not supported");
    }
    else if constexpr (std::is_null_pointer_v<T>) {
        static_assert(false, "nullptr_t not supported");
    }
    else if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            return {start_time, {name, T_SIGNED_INTEGRAL, type_i, val, sizeof(T)}};
        } else {
            return {start_time, {name, T_U_INTEGRAL, type_i, val, sizeof(T)}};
        }
    }
    else if constexpr (std::is_floating_point_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            return {start_time, {name, T_SIGNED_FLOAT, type_i, val, sizeof(T)}};
        } else {
            return {start_time, {name, T_U_FLOAT, type_i, val, sizeof(T)}};
        }
    }
    else if constexpr (is_std_array<T>::value) {
        static_assert(false, "arrays are not supported: please consider flattening the type");
    }
    else if constexpr (is_vector<T>::value) {
        static_assert(false, "vectors are not supported: please consider flattening the type");
    }
    else if constexpr (is_list<T>::value) {
        static_assert(false, "lists are not supported: please consider flattening the type");
    }
    else if constexpr (std::is_array_v<T>) {
        static_assert(false, "C-arrays are not supported: please consider flattening the type");
    }
    else if constexpr (std::is_enum_v<T>) {
        auto color_entries = magic_enum::enum_entries<T>();
        std::vector<std::pair<std::string, long long>> tmp_vals;
        static_assert(MAGIC_ENUM_RANGE_MIN > static_cast<uint64_t>(std::numeric_limits<T>::min()));
        static_assert(MAGIC_ENUM_RANGE_MAX >= static_cast<uint64_t>(std::numeric_limits<T>::min()));
        tmp_vals.reserve(color_entries.size());
        for (const auto& [enum_val, enum_string] : color_entries) {
            tmp_vals.emplace_back(std::string(enum_string), (uint64_t)(enum_val));
        }
        return {start_time, {name, T_ENUM, type_i, val, sizeof(T)}};
        // return  nullptr; //EnumField::from_type<T>((getter));
    }
    else if constexpr (is_tuple<T>::value) {
        static_assert(false, "tuples are not supported: please consider flattening the type");
    }
    else if constexpr (std::is_union_v<T>) {
        static_assert(false, "unions are not supported: please consider flattening the type");
    }
    else if constexpr (is_smart_pointer<T>::value) {
        static_assert(false, "smart pointers are not supported: please consider flattening the type");
    }
    else if constexpr (is_variant<T>::value) {
        static_assert(false, "variants are not supported: please consider flattening the type");
    }
    else if constexpr (std::is_function_v<T>) {
        static_assert(false, "functions are not supported: please consider flattening the type");
    }
    else if constexpr (is_actual_pointer<T>::value) {
        static_assert(false, "pointers are not supported: please consider flattening the type");
    }
    else if constexpr (std::is_class_v<T>) {
        static_assert(false, "unions are not supported: please consider flattening the type");
    } else {
        static_assert(false, "unsupported unknown type case");
    }
}

#endif //GENERALFRAMEWORK_ANYFUNDAMENTALVARIABLEMONITORING_H