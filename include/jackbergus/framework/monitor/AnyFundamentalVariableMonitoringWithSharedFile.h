// AnyFundamentalVariableMonitoringWithSharedFile.h
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
//
// Created by Giacomo Bergami on 29/04/2026.
//

#ifndef GENERALFRAMEWORK_ANYFUNDAMENTALVARIABLEMONITORINGWITHSHAREDFILE_H
#define GENERALFRAMEWORK_ANYFUNDAMENTALVARIABLEMONITORINGWITHSHAREDFILE_H

#include <cstdint>
#include <functional>
#include <jackbergus/framework/types/NativeTypes.h>
#include <narcissus/lightweight_any.h>
#include <jackbergus/framework/monitor/NativeTypeMonitoring.h>
#include <jackbergus/framework/ndp/FileSerializer.h>
#include <refl.hpp>

#include "AnyFundamentalVariableMonitoring.h"
#include "jackbergus/data/recursive_encoder.h"

using stack_function = std::function<lightweight_any(const lightweight_any&)>;

/**
 * AnyFundamentalVariableMonitoringWithSharedFile is basically a specification of AnyFundamentalVariableMonitoring, with the only
 * main difference that the file is shared across the fields rather than being unique to a specific field, forsooth
 *
 * @tparam block_size Maximum size block that will be stored for flushing on disk
 */
template<uint64_t block_size = 1024>
class AnyFundamentalVariableMonitoringWithSharedFile {
    jackbergus::framework::FinestScaleTimeRepresentation start_time, end_time_inclusive;
    lightweight_any current_value;
    bool is_value_valid;
    jackbergus::framework::NativeTypeMonitoring type_info;
    uint64_t struct_idx, field_idx;
    std::optional<jackbergus::framework::new_delta_data_structure> buffer;
    std::weak_ptr<jackbergus::framework::FileSerializer<block_size> > file_serialized;
    bool is_bitfield_;
    uint64_t bit_offset_;
    uint64_t bit_size_;
    static_assert(sizeof(jackbergus::framework::new_delta_data_structure) + 1 < block_size,
                  "new_delta_data_structure size mismatch");
    std::vector<stack_function> stacks;

public:

    void addStackFunction(stack_function&& function) {
        stacks.emplace_back(std::move(function));
    }
    const std::vector<stack_function>& getStack() {
        return stacks;
    }
    bool isBitfield() const {
        return is_bitfield_;
    }

    uint64_t bitOffset() const {
        return bit_offset_;
    }

    uint64_t bitSize() const {
        return bit_size_;
    }

    void flush(uint64_t min_idx,
               uint64_t max_id,
               bool lastWrite = false,
               bool firstWrite = false) {
        if (buffer.has_value()) {
            if (auto ptr = file_serialized.lock()) {
                auto &record = buffer.value();
                //for (auto& record : variables) {
                if ((record.timestamp == 0.0) && (!firstWrite)) {
                    buffer = {};
                    return;
                }
                bool isMinOrMax = false;
                if (lastWrite || (record.unnested_field_id == min_idx)) {
                    isMinOrMax = true;
                    record.is_starting_of_structure = 1;
                } else {
                    record.is_starting_of_structure = 0;
                }
                if (lastWrite || (record.unnested_field_id == max_id)) {
                    isMinOrMax = true;
                    record.is_end_of_structure = 1;
                } else {
                    record.is_end_of_structure = 0;
                }
                if (lastWrite || !isMinOrMax) {
                    record.is_continuing_of_structure = 1;
                } else {
                    record.is_continuing_of_structure = 0;
                }
                ptr->write((void *) &record, sizeof(record));
                buffer = {};
            }
        }
    }

    std::ostream& printNative(std::ostream& f, const lightweight_any & finale_wrapped) {
        switch (type_info.casusu) {
            case T_VOID:
                return f << "void";

            case T_NULLPTR:
                return f << "nullptr";

            case T_SIGNED_INTEGRAL:
                if (type_info.sizeof_ == 1) {
                    return f << (int64_t)*((int8_t*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == 2) {
                    return f << (int64_t)*((int16_t*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == 4) {
                    return f << (int64_t)*((int32_t*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == 8) {
                    return f << (int64_t)*((int64_t*)finale_wrapped.raw());
                } else
                    return f << "??si";

            case T_U_INTEGRAL:
                if (type_info.sizeof_ == 1) {
                    return f << (uint64_t)*((uint8_t*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == 2) {
                    return f << (uint64_t)*((uint16_t*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == 4) {
                    return f << (uint64_t)*((uint32_t*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == 8) {
                    return f << (uint64_t)*((uint64_t*)finale_wrapped.raw());
                } else
                    return f << "??ui";
                break;
            case T_SIGNED_FLOAT:
            case T_U_FLOAT:
                if (type_info.sizeof_ == sizeof(float)) {
                    return f << *((float*)finale_wrapped.raw());
                } else if (type_info.sizeof_ == sizeof(double)) {
                    return f << *((double*)finale_wrapped.raw());
                } else
                    return f << "??f";

            case T_STATIC_ARRAY:
                return f << "array?";

            case T_OTHER_ARRAY:
                return f << "oth_array?";

            case T_ENUM:
                return f << *(uint64_t*)finale_wrapped.raw();

            case T_UNION:
                return f << "union?";

            case T_CLASS:
                return f << "clz?";

            case T_FUNCTION:
                return f << "fun?";

            case T_POINTER:
                return f << "ptr?";

            case T_STRING:
                return f << "str?";

            case T_TUPLE:
                return f << "prod?";

            case T_VARIANT:
                return f << "union?";

            case T_UNEXPECTED:
                return f << "wtf?";

        }
    }

    AnyFundamentalVariableMonitoringWithSharedFile
    (std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &file_serialized,
     uint64_t struct_idx,
     jackbergus::framework::FinestScaleTimeRepresentation start_time,
     jackbergus::framework::NativeTypeMonitoring &&type,
     bool is_bitfield,
     uint64_t bit_offset,
     uint64_t bit_size) : start_time(start_time), type_info{std::move(type)},
                                                           end_time_inclusive(start_time), is_value_valid(false),
                                                           file_serialized{file_serialized}, struct_idx{struct_idx},
                                                           field_idx{std::numeric_limits<uint64_t>::max()},
                                                           is_bitfield_(is_bitfield), bit_offset_(bit_offset),
                                                           bit_size_(bit_size) {
    }

    AnyFundamentalVariableMonitoringWithSharedFile
    (std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &file_serialized,
     uint64_t struct_idx,
     jackbergus::framework::FinestScaleTimeRepresentation start_time,
     jackbergus::framework::NativeTypeMonitoring &&type,
     const lightweight_any &value,
     bool is_bitfield,
     uint64_t bit_offset,
     uint64_t bit_size) : type_info{std::move(type)}, start_time(start_time), end_time_inclusive(start_time),
                                     current_value(value), is_value_valid(true), file_serialized{file_serialized},
    struct_idx{struct_idx}, field_idx{std::numeric_limits<uint64_t>::max()},
                          is_bitfield_(is_bitfield), bit_offset_(bit_offset),
                          bit_size_(bit_size) {
    }

    AnyFundamentalVariableMonitoringWithSharedFile(const AnyFundamentalVariableMonitoringWithSharedFile &other)
    = default;

    AnyFundamentalVariableMonitoringWithSharedFile(AnyFundamentalVariableMonitoringWithSharedFile &&other) = default;

    AnyFundamentalVariableMonitoringWithSharedFile &operator=(
        const AnyFundamentalVariableMonitoringWithSharedFile &other) = default;

    AnyFundamentalVariableMonitoringWithSharedFile &operator=(AnyFundamentalVariableMonitoringWithSharedFile &&other)
    = default;

    ~AnyFundamentalVariableMonitoringWithSharedFile() = default;

    const std::string &field_name() const {
        return type_info.name;
    }

    type_cases field_type() const {
        return type_info.casusu;
    }

    uint64_t native_size() const {
        return type_info.sizeof_;
    }

    std::unordered_map<uint64_t, std::string> enum_info() const {
        return type_info.forEnumValueToNameMapping;
    }

#if __cplusplus >= 202302L
    const std::type_index &type() const {
        return type_info.type_i;
    }
#endif

    void finalizeFile() {
        if (auto tmp = file_serialized.lock()) {
            flush(0, 0, true);
            if (is_value_valid) {
                start_time = end_time_inclusive = 0;
                is_value_valid = false;
            }
        }
    }

    bool isCurrentlyValid() const {
        return is_value_valid;
    }

    std::pair<jackbergus::framework::FinestScaleTimeRepresentation,
        jackbergus::framework::FinestScaleTimeRepresentation> validityInterval() const {
        if (is_value_valid) {
            return {start_time, end_time_inclusive};
        } else {
            return {-1, -1};
        }
    }

    /**
     *
     * @tparam T
     * @return
     */
    template<typename T>
    T *get() const {
        return is_value_valid ? (T *) current_value.raw() : nullptr;
    }

    [[nodiscard]] const jackbergus::framework::FinestScaleTimeRepresentation getCurrentTime() const {
        return end_time_inclusive;
    }

    [[nodiscard]] void *getRawPtr() const {
        return is_value_valid ? (void *) current_value.raw() : nullptr;
    }

    bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                     const lightweight_any &value,
                     uint64_t field_idx) {
        return updateValue(curr_t, value, {}, field_idx);
    }

    /**
     *
     * @param curr_t Time when the value was recorded
     * @param value Value being recorded at the current time
     * @param t Optional equality predicate. If missing, it is using the default one
     * @return Whether the value is ready to be persisted on disk, so to force the writing, forsooth!
     */
    bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                     const lightweight_any &value,
                     const std::equal_to<lightweight_any> &t,
                     uint64_t field_idx) {
        ;
        bool isFirstSet = false;
        if (this->field_idx == std::numeric_limits<uint64_t>::max()) {
            this->field_idx = field_idx;
            isFirstSet = true;
        }
        if ((end_time_inclusive > curr_t) || (this->field_idx != field_idx)) {
            return false;
        }
        if (!is_value_valid) {
            start_time = end_time_inclusive = curr_t;
            current_value = value;
            is_value_valid = true;
            jackbergus::framework::new_delta_data_structure s; // TODO
            s.timestamp = this->start_time;
            s.structure_id = this->struct_idx;
            s.actual_type = type_info.casusu;
            s.unnested_field_id = this->field_idx;
            s.actual_size = type_info.sizeof_;
            // start, end, continuing are just determined at the final finalization, forsooth!
            s.actual_data = *(uint64_t *) current_value.raw();
            s.setCRC();
            buffer = {s};
            if (!isFirstSet)
                return false;
        }
        if ((!isFirstSet) && t(current_value, value)) {
            end_time_inclusive = curr_t;
            return false;
        } else {
            //pushRecord(s);
            start_time = end_time_inclusive = curr_t;
            current_value = value;
            is_value_valid = true;
            jackbergus::framework::new_delta_data_structure s; // TODO
            s.timestamp = this->start_time;
            s.structure_id = this->struct_idx;
            s.actual_type = type_info.casusu;
            s.unnested_field_id = this->field_idx;
            s.actual_size = type_info.sizeof_;
            // start, end, continuing are just determined at the final finalization, forsooth!
            s.actual_data = *(uint64_t *) current_value.raw();
            s.setCRC();
            buffer = {s};
            return true;
        }
    }
};

template<typename T, uint64_t block_size = 1024>
AnyFundamentalVariableMonitoringWithSharedFile<block_size> flatten_type_to_enum2( //start_time, idx, x, base_path+view
                                                                std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &fileptr,
                                                                jackbergus::framework::FinestScaleTimeRepresentation start_time,
                                                                uint64_t idx,
                                                                uint64_t val,
                                                                const std::string &name,
                                                                bool is_bitfield,
                                                                size_t bit_offset,
                                                                size_t bit_size) {
    std::type_index type_i = std::type_index(typeid(T));
    if constexpr (std::is_same_v<T, std::string>) {
        static_assert(false, "std::string not supported");
    }
    if constexpr (std::is_void_v<T>) {
        static_assert(false, "void not supported");
    } else if constexpr (std::is_null_pointer_v<T>) {
        static_assert(false, "nullptr_t not supported");
    } else if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            return {fileptr, idx, start_time, {name, T_SIGNED_INTEGRAL, type_i, val, sizeof(T)}, is_bitfield, bit_offset, bit_size};
        } else {
            return {fileptr, idx, start_time, {name, T_U_INTEGRAL, type_i, val, sizeof(T)}, is_bitfield, bit_offset, bit_size};
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            return {fileptr, idx, start_time, {name, T_SIGNED_FLOAT, type_i, val, sizeof(T)}, is_bitfield, bit_offset, bit_size};
        } else {
            return {fileptr, idx, start_time, {name, T_U_FLOAT, type_i, val, sizeof(T)}, is_bitfield, bit_offset, bit_size};
        }
    } else if constexpr (is_std_array<T>::value) {
        static_assert(false, "arrays are not supported: please consider flattening the type");
    } else if constexpr (is_vector<T>::value) {
        static_assert(false, "vectors are not supported: please consider flattening the type");
    } else if constexpr (is_list<T>::value) {
        static_assert(false, "lists are not supported: please consider flattening the type");
    } else if constexpr (std::is_array_v<T>) {
        static_assert(false, "C-arrays are not supported: please consider flattening the type");
    } else if constexpr (std::is_enum_v<T>) {
        auto color_entries = magic_enum::enum_entries<T>();
        static_assert(MAGIC_ENUM_RANGE_MIN > static_cast<uint64_t>(std::numeric_limits<T>::min()));
        static_assert(MAGIC_ENUM_RANGE_MAX >= static_cast<uint64_t>(std::numeric_limits<T>::min()));
        std::unordered_map<uint64_t, std::string> forEnumValueToNameMapping;
        for (auto &val_entry: magic_enum::enum_values<T>()) {
            std::string enum_name = magic_enum::enum_name(val_entry).data();
            forEnumValueToNameMapping[static_cast<uint64_t>(val_entry)] = enum_name;
        }
        return {fileptr, idx, start_time, {name, T_ENUM, type_i, val, sizeof(T), forEnumValueToNameMapping}, is_bitfield, bit_offset, bit_size};
    } else if constexpr (is_tuple<T>::value) {
        static_assert(false, "tuples are not supported: please consider flattening the type");
    } else if constexpr (std::is_union_v<T>) {
        static_assert(false, "unions are not supported: please consider flattening the type");
    } else if constexpr (is_smart_pointer<T>::value) {
        static_assert(false, "smart pointers are not supported: please consider flattening the type");
    } else if constexpr (is_variant<T>::value) {
        static_assert(false, "variants are not supported: please consider flattening the type");
    } else if constexpr (std::is_function_v<T>) {
        static_assert(false, "functions are not supported: please consider flattening the type");
    } else if constexpr (is_actual_pointer<T>::value) {
        static_assert(false, "pointers are not supported: please consider flattening the type");
    } else if constexpr (std::is_class_v<T>) {
        static_assert(false, "classes are not supported: please consider flattening the type");
    } else {
        static_assert(false, "unsupported unknown type case");
    }
}



template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_forshared {
    static std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> > expandWithBasicMonitor(
                    std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &fileptr,
                    jackbergus::framework::FinestScaleTimeRepresentation start_time,
                    const std::string &base_path = "",
                    uint64_t idx = 0,
                    uint64_t prev_record_offset = 0) {

        uint64_t old_prev_record_offset = prev_record_offset;
        std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> > current;
        constexpr bool is_bitfield = refl::trait::get_t<x, refl::member_list<T> >::is_bitfield;
        constexpr uint64_t bit_offset = refl::trait::get_t<x, refl::member_list<T> >::bitfield_offset;
        constexpr uint64_t bit_size = refl::trait::get_t<x, refl::member_list<T> >::bitsize;
        auto view = std::string(refl::trait::get_t<x, refl::member_list<T> >::name.c_str());
        using K = typename refl::trait::get_t<x, refl::member_list<T> >::value_type;
        constexpr auto val = getTypeInformation<K>();
        if constexpr (val == type_cases::T_CLASS) {
            // if I am now dealing with a class, then consider recursively wrapping this into something else,
            // and re-start the computation back again
            current = static_forshared<K, 0, refl::member_list<K>::size>::expandWithBasicMonitor(
                fileptr, start_time, base_path + view + ".", idx, bit_offset);
        } else if constexpr (val == type_cases::T_STATIC_ARRAY) {
            // If this is an array, then I need to consider whether the internal element is a fundamental type or not
            using H = typename std::remove_all_extents_t<K>;
            constexpr auto current_size = sizeof(H);
            constexpr uint64_t N = sizeof(K) / current_size;
            if constexpr (!std::is_fundamental_v<H>) {
                constexpr auto current_packed_size = refl::descriptor::bit_val<H>();
                for (auto i = 0u; i < N; ++i) {
                    auto local = static_forshared<H, 0, refl::member_list<H>::size>::expandWithBasicMonitor(
                        fileptr, start_time, base_path + view + "[" + std::to_string(i) + "].", idx, prev_record_offset+bit_offset);
                    uint64_t array_idx = i;
                    for (auto& ref : local) {
                        ref.addStackFunction([array_idx](auto field_access1) {
                            auto ptr2 = (T*)field_access1.raw();
                            auto& ref_field3 = *ptr2.*(refl::trait::get_t<x, refl::member_list<T>>::pointer);
                            lightweight_any field_access2{&ref_field3[array_idx]};
                            return field_access2;
                        });
                    }
                    current.insert(current.end(), std::move_iterator(local.begin()), std::move_iterator(local.end()));
                    prev_record_offset += current_packed_size;
                }
            } else {
                for (auto i = 0u; i < N; ++i) {
                    current.emplace_back(flatten_type_to_enum2<H>(fileptr, start_time, idx, x,
                                                                  base_path + view + "[" + std::to_string(i) + "]", is_bitfield, prev_record_offset+bit_offset, bit_size));
                    prev_record_offset += current_size;
                    uint64_t idx_ = x;
                    uint64_t array_idx = i;
                    current.rbegin()->addStackFunction( [idx_, array_idx](auto field_access2) {
                        auto ref_field3 = getter<T, idx_>(*(T*)field_access2.raw());
                        return lightweight_any{ref_field3[array_idx]};
                    });
                }
            }
        } else {
            // Otherwise, just return the element as it stands, forsooth!
            current.emplace_back(flatten_type_to_enum2<K>(fileptr, start_time, idx, x, base_path + view, is_bitfield, prev_record_offset+bit_offset, bit_size));
            current.rbegin()->addStackFunction( [idx](auto field_access2) {
                auto ref_field3 = getter<T, x>(*(T*)field_access2.raw());
                return lightweight_any{ref_field3};
            });
        }
        auto result = static_forshared<T, x + 1, to>::expandWithBasicMonitor(fileptr, start_time, base_path, idx, old_prev_record_offset);
        current.insert(current.end(), std::move_iterator(result.begin()), std::move_iterator(result.end()));
        return current;
    }

    static uint64_t setRecursivelyWithTemplates(std::vector<uint64_t> &to_persist_fields,
                                                jackbergus::framework::FinestScaleTimeRepresentation start_time,
                                                const T &value,
                                                std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> >
                                                &f,
                                                uint64_t acc = 0) {
        auto val = getter<T, x>(value);
        using K = typename refl::trait::get_t<x, refl::member_list<T> >::value_type;
        constexpr auto t_val = getTypeInformation<K>();
        if constexpr (t_val == type_cases::T_STATIC_ARRAY) {
            using H = typename std::remove_all_extents_t<K>;
            constexpr uint64_t N = sizeof(K) / sizeof(H);
            if constexpr (!std::is_fundamental_v<H>) {
                for (auto i = 0u; i < N; i++) {
                    acc = static_forshared<H, 0, refl::member_list<H>::size>::setRecursivelyWithTemplates(
                        to_persist_fields, start_time, val[i], f, acc);
                }
                return static_forshared<T, x + 1, to>::setRecursivelyWithTemplates(
                    to_persist_fields, start_time, value, f, acc);
            } else {
                for (auto i = 0u; i < N; ++i) {
                    if (f[acc].updateValue(start_time, val[i], acc))
                        to_persist_fields.emplace_back(acc);
                    acc++;
                }
                return static_forshared<T, x + 1, to>::setRecursivelyWithTemplates(
                    to_persist_fields, start_time, value, f, acc);
            }
        } else if constexpr (t_val != type_cases::T_CLASS) {
            // if this is merely a field, then doing the immediate update
            if (f[acc].updateValue(start_time, val, acc))
                to_persist_fields.emplace_back(acc);
            return static_forshared<T, x + 1, to>::setRecursivelyWithTemplates(
                to_persist_fields, start_time, value, f, acc + 1);
        } else {
            // otherwise, I need to recursively analyse this by not updating the field directly, rather one of
            // its constituents
            auto withInDepthRecursion = static_forshared<K, 0, refl::member_list<K>::size>{}.
                    setRecursivelyWithTemplates(to_persist_fields, start_time, val, f, acc);
            return static_forshared<T, x + 1, to>::setRecursivelyWithTemplates(
                to_persist_fields, start_time, value, f, withInDepthRecursion);
        }
    }
};

template<typename T, int to, uint64_t block_size>
struct static_forshared<T, to, to, block_size> {
    static uint64_t setRecursivelyWithTemplates(std::vector<uint64_t> &to_persist_fields,
                                                jackbergus::framework::FinestScaleTimeRepresentation start_time,
                                                const T &value,
                                                std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> >
                                                &f,
                                                uint64_t acc = 0) {
        return acc;
    }

    static std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> >
    expandWithBasicMonitor(std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &fileptr,
                           jackbergus::framework::FinestScaleTimeRepresentation start_time,
                           const std::string &base_path = "",
                           uint64_t idx = 0,
                           uint64_t prev_offset = 0) {
        return {};
    }
};

template<typename T, uint64_t block_size = 1024>
std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> >
getNativeType2(std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &fileptr,
               jackbergus::framework::FinestScaleTimeRepresentation start_time,
               uint64_t idx) {
    auto v = static_forshared<T, 0, refl::member_list<T>::size>::expandWithBasicMonitor(fileptr, start_time, "", idx, 0);
    return v;
}

#endif //GENERALFRAMEWORK_ANYFUNDAMENTALVARIABLEMONITORINGWITHSHAREDFILE_H