//
// Created by Giacomo Bergami on 28/04/2026.
//

#include <cstdint>
#include <limits>

#include <magic_enum/magic_enum.hpp>
#include <narcissus/reflection/type_cases.h>

#include "admissible_nested_example.h"
#include "jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h"
#include "jackbergus/framework/monitor/AnyStructMonitoring.h"
#include "jackbergus/framework/ndp/FileBlockReader.h"




/**
 * AnyFundamentalVariableMonitoringWithSharedFile is basically a specification of AnyFundamentalVariableMonitoring, with the only
 * main difference that the file is shared across the fields rather than being unique to a specific field, forsooth
 *
 * @tparam block_size Maximum size block that will be stored for flushing on disk
 */
template <uint64_t block_size = 1024>
        class AnyFundamentalVariableMonitoringWithSharedFile {
            jackbergus::framework::FinestScaleTimeRepresentation start_time, end_time_inclusive;
            lightweight_any current_value;
            bool is_value_valid;
            jackbergus::framework::NativeTypeMonitoring type_info;
            uint64_t struct_idx, field_idx;
            std::optional<jackbergus::framework::new_delta_data_structure> buffer;
            std::weak_ptr<jackbergus::framework::FileSerializer<block_size>> file_serialized;
            static_assert(sizeof(jackbergus::framework::new_delta_data_structure)+1 < block_size, "new_delta_data_structure size mismatch");

        public:

            void flush(uint64_t min_idx,
                       uint64_t max_id,
                       bool lastWrite = false,
                       bool firstWrite = false) {
                if (buffer.has_value()) {
                    if (auto ptr = file_serialized.lock()) {
                         auto& record = buffer.value();
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
                            ptr->write((void*)&record, sizeof(record));
                            buffer = {};
                        }
                    }
            }

            AnyFundamentalVariableMonitoringWithSharedFile
                (std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& file_serialized,
                 uint64_t struct_idx,
                 jackbergus::framework::FinestScaleTimeRepresentation start_time,
                 jackbergus::framework::NativeTypeMonitoring&& type) : start_time(start_time), type_info{std::move(type)},  end_time_inclusive(start_time), is_value_valid(false), file_serialized{file_serialized}, struct_idx{struct_idx}, field_idx{std::numeric_limits<uint64_t>::max()} {}
            AnyFundamentalVariableMonitoringWithSharedFile
                (std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& file_serialized,
                 uint64_t struct_idx,
                 jackbergus::framework::FinestScaleTimeRepresentation start_time,
                 jackbergus::framework::NativeTypeMonitoring&& type,
                 const lightweight_any& value) : type_info{std::move(type)},  start_time(start_time), end_time_inclusive(start_time), current_value(value), is_value_valid(true), file_serialized{file_serialized}, struct_idx{struct_idx}, field_idx{std::numeric_limits<uint64_t>::max()} {}

            AnyFundamentalVariableMonitoringWithSharedFile(const AnyFundamentalVariableMonitoringWithSharedFile& other) = default;
            AnyFundamentalVariableMonitoringWithSharedFile(AnyFundamentalVariableMonitoringWithSharedFile&& other) = default;
            AnyFundamentalVariableMonitoringWithSharedFile& operator=(const AnyFundamentalVariableMonitoringWithSharedFile& other) = default;
            AnyFundamentalVariableMonitoringWithSharedFile& operator=(AnyFundamentalVariableMonitoringWithSharedFile&& other) = default;

            ~AnyFundamentalVariableMonitoringWithSharedFile()  = default;

            const std::string& field_name() const {
                return type_info.name;
            }

            type_cases field_type() const {
                return type_info.casusu;
            }

            uint64_t native_size() const {
                return type_info.sizeof_;
            }

#if __cplusplus >= 202302L
            const std::type_index& type() const {
                return type_info.type_i;
            }
#endif

            void finalizeFile()  {
                if (auto tmp = file_serialized.lock()) {
                    flush(0, 0, true);
                    if (is_value_valid) {
                        start_time = end_time_inclusive = 0;
                        is_value_valid = false;
                    }
                }
            }

            bool isCurrentlyValid() const  {
                return is_value_valid;
            }

            std::pair<jackbergus::framework::FinestScaleTimeRepresentation, jackbergus::framework::FinestScaleTimeRepresentation> validityInterval() const {
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

    [[nodiscard]] const jackbergus::framework::FinestScaleTimeRepresentation getCurrentTime() const  {
                return end_time_inclusive;
            }

    [[nodiscard]] void* getRawPtr() const  {
        return is_value_valid ? (void*)current_value.raw() : nullptr;
    }

    bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                             const lightweight_any& value,
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
                             const lightweight_any& value,
                             const std::equal_to<lightweight_any>& t,
                             uint64_t field_idx) {
;               bool isFirstSet = false;
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
                    s.actual_data = *(uint64_t*)current_value.raw();
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
                    s.actual_data = *(uint64_t*)current_value.raw();
                    s.setCRC();
                    buffer = {s};
                    return true;
                }
            }
        };


template <typename T, uint64_t block_size = 1024>
AnyFundamentalVariableMonitoringWithSharedFile<block_size> flatten_type_to_enum2( //start_time, idx, x, base_path+view
    std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& fileptr,
    jackbergus::framework::FinestScaleTimeRepresentation start_time,
    uint64_t idx,
    uint64_t val,
    const std::string& name) {

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
            return {fileptr, idx, start_time, {name, T_SIGNED_INTEGRAL, type_i, val, sizeof(T)}};
        } else {
            return {fileptr, idx, start_time, {name, T_U_INTEGRAL, type_i, val, sizeof(T)}};
        }
    }
    else if constexpr (std::is_floating_point_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            return {fileptr, idx, start_time, {name, T_SIGNED_FLOAT, type_i, val, sizeof(T)}};
        } else {
            return {fileptr, idx, start_time, {name, T_U_FLOAT, type_i, val, sizeof(T)}};
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
       // std::vector<std::pair<std::string, long long>> tmp_vals;
        static_assert(MAGIC_ENUM_RANGE_MIN > static_cast<uint64_t>(std::numeric_limits<T>::min()));
        static_assert(MAGIC_ENUM_RANGE_MAX >= static_cast<uint64_t>(std::numeric_limits<T>::min()));
        /*tmp_vals.reserve(color_entries.size());
        for (const auto& [enum_val, enum_string] : color_entries) {
            tmp_vals.emplace_back(std::string(enum_string), (uint64_t)(enum_val));
        }*/
        return {fileptr, idx, start_time, {name, T_ENUM, type_i, val, sizeof(T)}};
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
        static_assert(false, "classes are not supported: please consider flattening the type");
    } else {
        static_assert(false, "unsupported unknown type case");
    }
}


template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_forshared {

        std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>> expandWithBasicMonitor(
            std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& fileptr,
            jackbergus::framework::FinestScaleTimeRepresentation start_time,
            const std::string& base_path = "",
            uint64_t idx = 0) {
                std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>> current;
                auto view = std::string(refl::trait::get_t<x, refl::member_list<T>>::name.c_str());
                using K = typename refl::trait::get_t<x, refl::member_list<T>>::value_type;
                constexpr auto val = getTypeInformation<K>();
                if constexpr (val == type_cases::T_CLASS) {
                    // if I am now dealing with a class, then consider recursively wrapping this into something else,
                    // and re-start the computation back again
                    current = static_forshared<K, 0, refl::member_list<K>::size>{}.expandWithBasicMonitor(fileptr, start_time, base_path + view + ".", idx);
                } else if constexpr (val == type_cases::T_STATIC_ARRAY) {
                   // If this is an array, then I need to consider whether the internal element is a fundamental type or not
                   using H = typename std::remove_all_extents_t<K>;
                   constexpr uint64_t N = sizeof(K)/sizeof(H);
                   if constexpr (!std::is_fundamental_v<H>) {
                       for (auto i = 0u; i < N; ++i) {
                           auto local = static_forshared<H, 0, refl::member_list<H>::size>{}.expandWithBasicMonitor(fileptr, start_time, base_path+view+"["+std::to_string(i)+"].", idx);
                           current.insert(current.end(), std::move_iterator(local.begin()), std::move_iterator(local.end()));
                       }
                   } else {
                       for (auto i = 0u; i < N; ++i) {
                           current.emplace_back(flatten_type_to_enum2<H>(fileptr, start_time,  idx, x, base_path+view+"["+std::to_string(i)+"]"));
                       }
                   }
                } else {
                    // Otherwise, just return the element as it stands, forsooth!
                    current.emplace_back(flatten_type_to_enum2<K>(fileptr,  start_time, idx, x, base_path+view));
                }
                auto result = static_forshared<T, x+1,to>().expandWithBasicMonitor(fileptr, start_time, base_path, idx);
                current.insert(current.end(), std::move_iterator(result.begin()), std::move_iterator(result.end()));
                return current;
            }

    uint64_t setRecursivelyWithTemplates(std::vector<uint64_t>& to_persist_fields,
                                         jackbergus::framework::FinestScaleTimeRepresentation start_time,
                                        const T& value,
                                        std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>>& f,
                                        uint64_t acc = 0) {
        auto val = get_field_t<T, x>::get(value);
        using K = typename refl::trait::get_t<x, refl::member_list<T>>::value_type;
        constexpr auto t_val = getTypeInformation<K>();
        if constexpr (t_val == type_cases::T_STATIC_ARRAY) {
            using H = typename std::remove_all_extents_t<K>;
            constexpr uint64_t N = sizeof(K)/sizeof(H);
            if constexpr (!std::is_fundamental_v<H>) {
                for (auto i = 0u; i<N; i++) {
                    acc = static_forshared<H, 0, refl::member_list<H>::size>{}.setRecursivelyWithTemplates(to_persist_fields, start_time, val[i], f, acc);
                }
                return static_forshared<T, x+1,to>().setRecursivelyWithTemplates(to_persist_fields, start_time, value, f, acc);
            } else {
                for (auto i = 0u; i < N; ++i) {
                    if (f[acc].updateValue(start_time, val[i], acc))
                        to_persist_fields.emplace_back(acc);
                    acc++;
                }
                return static_forshared<T, x+1,to>().setRecursivelyWithTemplates(to_persist_fields, start_time, value, f, acc);
            }
        } else if constexpr (t_val != type_cases::T_CLASS) {
            // if this is merely a field, then doing the immediate update
            if (f[acc].updateValue(start_time, val, acc))
                to_persist_fields.emplace_back(acc);
            return static_forshared<T, x+1,to>().setRecursivelyWithTemplates(to_persist_fields, start_time, value, f, acc+1);
        } else {
            // otherwise, I need to recursively analyse this by not updating the field directly, rather one of
            // its constituents
            auto withInDepthRecursion = static_forshared<K, 0, refl::member_list<K>::size>{}.setRecursivelyWithTemplates(to_persist_fields, start_time, val, f, acc);
            return static_forshared<T, x+1,to>().setRecursivelyWithTemplates(to_persist_fields, start_time, value, f, withInDepthRecursion);
        }
    }

};

template<typename T, int to, uint64_t block_size>
struct static_forshared<T, to,to, block_size> {


    uint64_t setRecursivelyWithTemplates(std::vector<uint64_t>& to_persist_fields,
                                         jackbergus::framework::FinestScaleTimeRepresentation start_time,
                                         const T& value,
                                         std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>>& f,
                                         uint64_t acc = 0) {
        return acc;
    }

    std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>>
              expandWithBasicMonitor(std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& fileptr,
                                     jackbergus::framework::FinestScaleTimeRepresentation start_time,
                                     const std::string& base_path = "",
                                     uint64_t idx = 0) {
        return {};
    }
};

template <typename  T, uint64_t block_size=1024> std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>>
            getNativeType2(std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& fileptr,
                           jackbergus::framework::FinestScaleTimeRepresentation start_time,
                           uint64_t idx) {
    auto v = static_forshared<T, 0, refl::member_list<T>::size>{}.expandWithBasicMonitor(fileptr, start_time, "", idx);
    return v;
}

template <uint64_t block_size = 1024>
struct SpecificStructureSerialization {
    uint64_t struct_idx;
    std::string object_name;
    std::type_index type_index;
    std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size>> fields;

     SpecificStructureSerialization() : object_name(""), type_index(std::type_index(typeid(void))), fields{}, struct_idx{std::numeric_limits<uint64_t>::max()} {
    }

    template <typename T > void init(const std::string& object_name,
                                     const T& initial_value_expected,
                                     std::shared_ptr<jackbergus::framework::FileSerializer<block_size>>& fileptr,
                                     const uint64_t struct_idx) {
        this->object_name = object_name;
        type_index = (std::type_index(typeid(T)));
         this->struct_idx = struct_idx;
         fields = getNativeType2<T>(fileptr, 0, this->struct_idx);
         updateValue(0, initial_value_expected, true);
    }

    void finaliseWrite() {
         for (auto& ref : fields) {
             ref.finalizeFile();
         }
     }

    template<typename T>
    bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                     const T& value,
                     bool firstWrite = false)  {
        if (type_index == std::type_index(typeid(T))) {
            std::vector<uint64_t> to_persist_fields;
            to_persist_fields.reserve(fields.size());
            static_forshared<T, 0, refl::member_list<T>::size>{}.setRecursivelyWithTemplates(to_persist_fields, curr_t, value, fields, 0);
            if (!to_persist_fields.empty()) {
                auto min = *to_persist_fields.begin();
                auto max = *to_persist_fields.rbegin();
                for (const auto& idx : to_persist_fields) {
                    fields[idx].flush(min, max, false, firstWrite);
                }
            }
            return true;
        } else {
            return false;
        }
    }

};

template <uint64_t block_size = 1024>
struct FileLogger {
    std::shared_ptr<jackbergus::framework::FileSerializer<block_size>> unique_file_serializer;
    std::unordered_map<std::string, SpecificStructureSerialization<block_size>> allSerializersPerform;
    std::vector<std::string> structNames;

    FileLogger(const std::string& binary_filename) : unique_file_serializer{new jackbergus::framework::FileSerializer<block_size>(binary_filename)} {

    }

    template<typename T>
    bool registerObjectWithName(const std::string& struct_name, const T& initial_val_t0) {
        uint64_t currentStructIndex;
        auto it = allSerializersPerform.find(struct_name);
        if (it == allSerializersPerform.end()) {
            currentStructIndex = allSerializersPerform.size();
            allSerializersPerform[struct_name].init(struct_name, initial_val_t0, unique_file_serializer, currentStructIndex);
            structNames.emplace_back(struct_name);
            return true; // Structure registration is successful
        } else {
            return false; // Structure already exists. Cannot register it twice!
        }
    }

    template<typename T>
    bool updateStruct(const std::string& struct_name, double timestamp, const T& value) {
        auto it = allSerializersPerform.find(struct_name);
        if (it == allSerializersPerform.end()) {
            return false;
        } else {
            return it->second.updateValue(timestamp, value, false);
        }
    }

    [[nodiscard]] const char* getCFileName() const { return unique_file_serializer.getCFileName(); }
    [[nodiscard]] const std::string& getFileName() const { return unique_file_serializer.getFileName(); }
    void close() {
        for (auto& ref : allSerializersPerform) {
            ref.second.finaliseWrite();
        }
        unique_file_serializer->flush();
        unique_file_serializer->close();
    }
    void flush() { unique_file_serializer->flush(); }

    ~FileLogger() {
        unique_file_serializer->close();
    }
};





int main(void) {
    // Creating a new approach, which is more saving on the file end side
    // If I have multiple fields that I have to serialize, while I am not having a perfect information about everything,
    // I need the following information
    //  - At time zero, I need to know the basic data information, which is the initial information from which I am computing the delta
    //  - I am considering only native data types
    //  - I am considering pointwise events, thus data elements occurring at a given element and place in time
    // Thus now, I am serializing within the same file, contiguously, all the variables that are changing, rather than providing some validity time delta period.
    // Also, please observe that I can bring this reasoning to its extremes, by simply assuming that all the data structures for all the files are just written into one single file, so to avoid the possibility of the os mismanaging multiple small files containing almost no data at all.
    // Thus, I am simply pipelining the changes into one single file.

    // But first, I am giving some trivial example of how lightweight_any works
    // This enables any casting, while preserving the type information....!
    // Also, it enables storing fundamental types while returning the underlying data as expected.
    {
        lightweight_any out;
        {
            uint64_t valuer{123};
            lightweight_any val{valuer};
            out = val;
        }
        std::cout << *(uint64_t*)out.raw() << std::endl;
        {
            double valued{123.456};
            lightweight_any val{valued};
            out = val;
        }
        std::cout << *(double*)out.raw() << std::endl;
        {
            uint8_t valuer{90};
            lightweight_any val{valuer};
            out = val;
        }
        std::cout << *(uint8_t*)out.raw() << std::endl;
    }

    FileLogger<> logger_test{"some_binary_file.bin"};

    Final_N final_n;
    memset(&final_n, 0, sizeof(final_n));
    logger_test.registerObjectWithName("final_n_msg", final_n);

    BogusConcurrentDataRecord concurrent;
    memset(&concurrent, 0, sizeof(concurrent));
    logger_test.registerObjectWithName("concurrent", concurrent);

    concurrent.val = 1; // valori dal tempo 1
    logger_test.updateStruct("concurrent", 1, concurrent);

    // From now onwards, simulating some elements being changed
    // Starting with small changes
    final_n.enumerato = 123; // Valori dal tempo 2
    final_n.third = 86;      // Valori dal tempo 2
    logger_test.updateStruct("final_n_msg", 2, final_n);

    concurrent.timestamp = 2.0; // valori dal tempo 2
    logger_test.updateStruct("concurrent", 2, concurrent);

    concurrent.val = 3; // valori dal tempo 3
    logger_test.updateStruct("concurrent", 3, concurrent);

    // Now, considering nested changes, forsooth!
    final_n.first.cho = 5; // Valori dal tempo 4
    final_n.first.jes = 6; // Valori dal tempo 4
    final_n.first.val = 7; // Valori dal tempo 4
    logger_test.updateStruct("final_n_msg", 4, final_n);

    final_n.second[5].cho = 11; // Valori dal tempo 5
    logger_test.updateStruct("final_n_msg", 5, final_n);

    concurrent.timestamp = 4.0; // valori dal tempo 5
    logger_test.updateStruct("concurrent", 5, concurrent);

    final_n.second[6].cho = 13; // Valori dal tempo 6
    logger_test.updateStruct("final_n_msg", 6, final_n);

    final_n.second[7].cho = 19; // Valori dal tempo 7
    logger_test.updateStruct("final_n_msg", 7, final_n);

    final_n.first.voi_ = 8; // Valori dal tempo 8
    logger_test.updateStruct("final_n_msg", 8, final_n);

    concurrent.val = 9; // valori dal tempo 9
    logger_test.updateStruct("concurrent", 9, concurrent);

    logger_test.flush();
    logger_test.close();

    jackbergus::framework::FileBlockReader<> fbr;
    jackbergus::framework::FileBlockWrapper<> buffer;
    fbr.open("some_binary_file.bin");

    while (fbr.read(buffer, false)) {
        for (uint64_t i = 0, N = buffer.size(); i<N; i++) {
            auto ptr = buffer.getNewRecord(i);
            const auto& structure = logger_test.structNames.at(ptr->structure_id);
            auto it = logger_test.allSerializersPerform.find(structure);
            std::cout << "t=" << ptr->timestamp << " " << logger_test.structNames.at(ptr->structure_id) << "." << it->second.fields[ptr->unnested_field_id].field_name() << std::endl;
        }
    }
}