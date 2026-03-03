#include <iostream>

#include <random>

#include "jackbergus/framework/monitor/ContinuousVariableMonitoring.h"
#include "field_reflection.hpp"


#include <narcissus/reflection/type_cases.h>
#include <typeindex>

#include "narcissus/lightweight_any.h"
#include "narcissus/template_typing.h"

using AnyVariableMonitoring = jackbergus::framework::VariableMonitoring<lightweight_any>;



struct NativeTypeMonitoring {
    std::string name;
    type_cases casusu;
    std::type_index type_i;
    uint64_t idx, sizeof_;

    NativeTypeMonitoring(const NativeTypeMonitoring& ) = default;
    NativeTypeMonitoring(NativeTypeMonitoring&& ) = default;
    NativeTypeMonitoring& operator=(const NativeTypeMonitoring& ) = default;
    NativeTypeMonitoring& operator=(NativeTypeMonitoring&& ) = default;
    NativeTypeMonitoring(const std::string& name, type_cases casusu, const std::type_index& idx, uint64_t record_offset, uint64_t sizeof_) : name(name), casusu(casusu), type_i(idx), idx{record_offset}, sizeof_(sizeof_) {}
};


template <uint64_t block_size = 1024>
        class AnyFundamentalVariableMonitoring : public jackbergus::framework::ContinuousMonitoring {
            jackbergus::framework::FinestScaleTimeRepreentation start_time, end_time_inclusive;
            lightweight_any current_value;
            bool is_value_valid;
            NativeTypeMonitoring type_info;

            std::vector<AnyVariableMonitoring> variables;
            std::unique_ptr<jackbergus::framework::FileSerializer<block_size>> file_serialized;

            void _pushRecordNonRecursive(AnyVariableMonitoring&& obj) {
                if (file_serialized) {
                    typename jackbergus::framework::FileSerializer<block_size>::BlockHeader block{};
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

            void _pushRecordNonRecursive(const AnyVariableMonitoring& obj) {
                if (file_serialized) {
                    typename jackbergus::framework::FileSerializer<block_size>::BlockHeader block{};
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

            void pushRecord(const AnyVariableMonitoring& obj) {
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

            void pushRecord(AnyVariableMonitoring& obj) {
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

            void clearFile() override {
                if (file_serialized) {
                    file_serialized->close();
                    file_serialized = nullptr;
                }
            }

            void setFile(const std::string& FileName) override {
                clearFile();
                file_serialized = std::make_unique<jackbergus::framework::FileSerializer<>>(FileName);
            }

            bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t) {
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

            bool isCurrentlyValid() const {
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


            /**
             *
             * @param curr_t Time when the value was recorded
             * @param value Value being recorded at the current time
             * @param t Optional equality predicate. If missing, it is using the default one
             * @return Whether the value was successfully updated
             */
            bool updateValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t,
                             const lightweight_any& value,
                             const std::equal_to<lightweight_any>& t = {}) {
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

#include <magic_enum/magic_enum.hpp>

template <typename T, uint64_t block_size = 1024> AnyFundamentalVariableMonitoring<block_size> flatten_type_to_enum(jackbergus::framework::FinestScaleTimeRepreentation start_time, uint64_t val, const std::string& name) {
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
        assert(MAGIC_ENUM_RANGE_MAX >= static_cast<uint64_t>(std::numeric_limits<T>::min()));
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

template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_for {
    std::vector<AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
        auto result = static_for<T, x+1,to>().expandWithBasicMonitor(start_time);
        auto view = field_reflection::field_name<T, x>;
        result.emplace_back(flatten_type_to_enum<field_reflection::field_type<T, x>>(start_time, x, std::string(view.data(), view.size())));
        return result;
    }
};

template<typename T, int to, uint64_t block_size>
struct static_for<T, to,to, block_size> {
    std::vector<AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
        return {};
    }
};

struct ExampleStructure {
    uint64_t a, b, c, d, e;
};

#include <algorithm>

template <typename  T, uint64_t block_size=1024> std::vector<AnyFundamentalVariableMonitoring<block_size>> getNativeType(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
    auto v = static_for<T, 0, field_reflection::field_count<T>>{}.expandWithBasicMonitor(start_time);
    std::reverse(v.begin(), v.end());
    return v;
}

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    auto s = getNativeType<ExampleStructure>(0);
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the <b>lang</b> variable name to see how CLion can help you rename it.
    double min = 10.0;
    double max = 50.0;
    AnyFundamentalVariableMonitoring counter = flatten_type_to_enum<double>(0, 0, "aDouble");
    // counter.setFile("binary.bin");

    std::random_device rd; // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> dista(min, max); // Inclusive range
    std::uniform_int_distribution<int> dist(0, 1);

    for (uint64_t idx = 0; idx < 10000; ++idx) {
        bool randomBool = static_cast<bool>(dist(gen));
        auto randomNum = dista(gen);

        if (randomBool) {
            std::cout << idx << ": "  << randomNum << std::endl;
            counter.updateValue(idx, randomNum);
            assert(*counter.get<double>() == randomNum);
        } else {
            std::cout << idx  << " Invalid " << std::endl;
            counter.setInvalidValue(idx);
        }
    }


    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}