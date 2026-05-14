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
    IntervalTree<uint64_t, uint64_t> interval_of_offsets;

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
        {
            std::vector<uint64_t> to_persist_fields;
            to_persist_fields.reserve(fields.size());
            static_forshared<T, 0, refl::member_list<T>::size>::setRecursivelyWithTemplates(
                to_persist_fields, 0.0, initial_value_expected, fields, 0);
            if (!to_persist_fields.empty()) {
                auto min = *to_persist_fields.begin();
                auto max = *to_persist_fields.rbegin();
                for (const auto &idx: to_persist_fields) {
                    fields[idx].flush(min, max, false, true);
                }
            }
        }
        struct_size = sizeof(T);
        fields_bearing.resize(struct_size, 0);
        std::memcpy(fields_bearing.data(), &initial_value_expected, sizeof(T));
        arbitrary_bitset t((unsigned char*)fields_bearing.data(), sizeof(T)*8);
        bsw = t;
        for (uint64_t idx = 0, N = fields.size(); idx < N; idx++) {
            const auto& field = fields[idx];
            // std::cout << "[" << field.bitOffset() << ", " << field.bitOffset()+field.bitSize()-1 << "] for " << field.field_name() <<  std::endl;
            interval_of_offsets.insertInterval({field.bitOffset(), field.bitOffset()+field.bitSize()-1, idx});
            //debug_fieldname_to_vectoroffset[field.field_name()] = idx;
        }
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
            // Old time consuming way to compute the delta writing:
            // This was actually recursing over the field data structures,
            // storing the values for each element, and determining whether
            // the previously stored value was updated or not. Then, I was
            // serializing the value on disk
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
            // In this new version of the algorithm, the thing that is performed is
            //
            // - Performing an efficient bitset operation to find the differences. So, I am comparing the data
            //   bitwise, and not necessarily considering the actual semantic representation. The former suffices
            //   to tell that the value changes by Leibniz Equality
            // - Given the bits that were changed, then I reconstruct which fields are of interest, and which were
            //   actually changed.
            // - To efficiently access the structure, I avoid a complicated and laggy recursive visit of the fields,
            //   but I directly use the bitmask to do so.

            // Please observe that I cannot just retain the delta and avoid storing the entire
            // data so to compare the delta, as it does not make sense. Also, the mechanism of just
            // updating the needed fields is possible if you have complete control of the updates, but
            // this is not general enough to consider external changes. Thus, you are resorted to
            // 1)

            arbitrary_bitset wrapper2((unsigned char*)&value, sizeof(T)*8);
            // std::cout<< bsw.toString() << std::endl;
            // std::cout << wrapper2.toString() << std::endl;
            // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            auto differences = bsw.deltaFromIntervalTreeSlot(interval_of_offsets, wrapper2);
            auto min = *differences.begin();
            auto max = *differences.rbegin();
            for (const auto idx : differences) {
                auto& field = fields[idx];
#ifdef FIRST_VERSION
                const auto& s = field.getStack();
                lightweight_any finale_wrapped{&value};
                for (auto it = s.rbegin(); it != s.rend(); it++) {
                    finale_wrapped = (*it)(finale_wrapped);
                }
                // field.printNative(std::cout, finale_wrapped) << " is the value changed for: " << field.field_name() << std::endl;
                field.updateValue(curr_t, finale_wrapped, idx);
#else
                // In this second optimization, please observe that the updateValue doesn't really care about the type
                // information, as this is retained within the field property alongside its actual size. Thus, I am
                // not required to recursively visit a stack as in the first implementation of this, and I can just
                // use as getter does and obtain the data, forsooth!
                bsw.clear();
                bsw.set_mask(bit_fill(field.bitSize()), field.bitOffset());
                bsw &= wrapper2;
                bsw >>= field.bitOffset();
                // std::cout << map.toString() << std::endl;
                uint64_t result = 0;
                memcpy(&result, bsw.bitset, std::min((field.bitSize()/sizeof(arbitrary_bitset::T)) + (field.bitSize()%sizeof(arbitrary_bitset::T) ? 1 : 0), sizeof(uint64_t)));
                field.updateValue(curr_t, result, idx);
#endif
                field.flush(min, max, false, firstWrite);
            }
            std::memcpy(fields_bearing.data(), &value, sizeof(T));
#endif

            // bsw = wrapper2;
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