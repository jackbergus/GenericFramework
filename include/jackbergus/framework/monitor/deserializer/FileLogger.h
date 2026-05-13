//
// Created by Giacomo Bergami, PhD on 29/04/2026.
//

#ifndef GENERALFRAMEWORK_FILELOGGER_H
#define GENERALFRAMEWORK_FILELOGGER_H

#include <fkYAML/node.hpp>
#include <cstdint>
#include <string>
#include <typeindex>
#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoringWithSharedFile.h>



template<uint64_t block_size = 1024>
struct SpecificStructureSerialization {
    uint64_t struct_idx;
    std::string object_name;
    std::type_index type_index;
    std::vector<AnyFundamentalVariableMonitoringWithSharedFile<block_size> > fields;
    uint64_t struct_size;
    std::vector<unsigned char> fields_bearing;
    arbitrary_bitset bsw;

    SpecificStructureSerialization() : object_name(""), type_index(std::type_index(typeid(void))), fields{},
                                       struct_idx{std::numeric_limits<uint64_t>::max()} {
    }

    template<typename T>
    void init(const std::string &object_name,
              const T &initial_value_expected,
              std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > &fileptr,
              const uint64_t struct_idx) {
        this->object_name = object_name;
        type_index = (std::type_index(typeid(T)));
        this->struct_idx = struct_idx;
        fields = getNativeType2<T>(fileptr, 0, this->struct_idx);
        updateValue(0, initial_value_expected, true);
        struct_size = sizeof(T);
        fields_bearing.resize(struct_size, 0);
        std::memcpy(fields_bearing.data(), &initial_value_expected, sizeof(T));
        arbitrary_bitset t((unsigned char*)fields_bearing.data(), sizeof(T));

    }

    void finaliseWrite() {
        for (auto &ref: fields) {
            ref.finalizeFile();
        }
    }

    template<typename T>
    bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                     const T &value,
                     bool firstWrite = false) {
        if (type_index == std::type_index(typeid(T)) && (struct_size == sizeof(T))) {
#ifdef OLD_DELTA
            // Old time consuming way to compute the delta writing: actually
            std::vector<uint64_t> to_persist_fields;
            to_persist_fields.reserve(fields.size());
            static_forshared<T, 0, refl::member_list<T>::size>::setRecursivelyWithTemplates(
                to_persist_fields, curr_t, value, fields, 0);
            if (!to_persist_fields.empty()) {
                auto min = *to_persist_fields.begin();
                auto max = *to_persist_fields.rbegin();
                for (const auto &idx: to_persist_fields) {
                    fields[idx].flush(min, max, false, firstWrite);
                }
            }
#else
#endif

            return true;
        } else {
            return false;
        }
    }
};

template<uint64_t block_size = 1024>
struct FileLogger {
    std::shared_ptr<jackbergus::framework::FileSerializer<block_size> > unique_file_serializer;
    std::unordered_map<std::string, SpecificStructureSerialization<block_size> > allSerializersPerform;
    std::vector<std::string> structNames;

    FileLogger(const std::string &binary_filename) : unique_file_serializer{
        new jackbergus::framework::FileSerializer<block_size>(binary_filename)
    } {
    }

    template<typename T>
    bool registerObjectWithName(const std::string &struct_name, const T &initial_val_t0) {
        uint64_t currentStructIndex;
        auto it = allSerializersPerform.find(struct_name);
        if (it == allSerializersPerform.end()) {
            currentStructIndex = allSerializersPerform.size();
            allSerializersPerform[struct_name].init(struct_name, initial_val_t0, unique_file_serializer,
                                                    currentStructIndex);
            structNames.emplace_back(struct_name);
            return true; // Structure registration is successful
        } else {
            return false; // Structure already exists. Cannot register it twice!
        }
    }

    void write_yaml_configuration() {
        auto FileName_ = getFileName();
        // Writing the yaml configuration
        fkyaml::node root = {{"binary", FileName_}, {"structs", fkyaml::node::mapping()}};
        auto &struct_map = root["structs"].as_map();

        for (uint64_t struct_id = 0, N = structNames.size(); struct_id < N; ++struct_id) {
            const auto &struct_name = structNames[struct_id];
            auto it = allSerializersPerform.find(struct_name);
            if (it != allSerializersPerform.end()) {
                auto &fields_bearing = it->second;
                fkyaml::node node = {{"name", struct_name}, {"fields", fkyaml::node::mapping()}, {"index", struct_id}};
                auto &field_struct = node["fields"].as_map();
                for (uint64_t field_id = 0, M = fields_bearing.fields.size(); field_id < M; ++field_id) {
                    auto &ref = fields_bearing.fields[field_id];
                    std::string val = ref.field_name();
                    auto struct_field = fkyaml::node::mapping();
                    struct_field["field_idx"] = field_id;
                    struct_field["field_name"] = val;
                    auto fieldo = ref.field_type();
                    struct_field["field_type"] = std::string(magic_enum::enum_name(fieldo));
                    struct_field["field_type_native_size"] = ref.native_size();
                    if (fieldo == type_cases::T_ENUM) {
                        auto yaml_info = fkyaml::node::mapping();
                        for (const auto &[idx, name]: ref.enum_info()) {
                            yaml_info[idx] = name;
                        }
                        struct_field["field_enum_info"] = yaml_info;
                    }
                    field_struct[val] = struct_field;
                }
                struct_map[struct_name] = node;
            }
        }
        if (!FileName_.empty()) {
            std::ofstream f{FileName_ + ".yaml"};
            f << root << std::endl;
        }
    }

    template<typename T>
    bool updateStruct(const std::string &struct_name, double timestamp, const T &value) {
        auto it = allSerializersPerform.find(struct_name);
        if (it == allSerializersPerform.end()) {
            return false;
        } else {
            return it->second.updateValue(timestamp, value, false);
        }
    }

    [[nodiscard]] const char *getCFileName() const { return unique_file_serializer->getCFileName(); }
    [[nodiscard]] const std::string getFileName() const { return unique_file_serializer->getFileName(); }

    void close() {
        for (auto &ref: allSerializersPerform) {
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

#endif //GENERALFRAMEWORK_FILELOGGER_H