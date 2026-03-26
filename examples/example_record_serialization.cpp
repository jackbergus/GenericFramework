#include <iostream>

#include <random>
#include <set>

#include <jackbergus/framework/ndp/AnyStructMonitoring.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>

#include <fkYAML/node.hpp>

struct ExampleStructure {
    uint64_t a, b, c, d, e;
};

REFL_AUTO(type(ExampleStructure), field(a), field(b), field(c), field(d), field(e))


struct FieldGetting {
    std::string field_name;
    type_cases field_type;
    uint64_t field_value_size;

    FieldGetting(const std::string &field_name, type_cases field_type, uint64_t field_value_size)
        : field_name(field_name),
          field_type(field_type),
          field_value_size(field_value_size) {
    }

    FieldGetting() : field_name(""), field_type(T_UNEXPECTED), field_value_size{0} {
    }

    FieldGetting(const FieldGetting &) = default;

    FieldGetting(FieldGetting &&) = default;

    FieldGetting &operator=(const FieldGetting &) = default;

    FieldGetting &operator=(FieldGetting &&) = default;
};

struct RecordFileDeserializer {
    RecordFileDeserializer() {
    }

    const std::vector<std::string> &columns() const {
        return field_names;
    }

    const uint64_t n_variables() const {
        return field_names.size();
    }

    const FieldGetting *getFieldInfo(uint64_t idx) const {
        if (idx >= fields.size()) {
            return nullptr;
        }
        return &fields[idx];
    }

    void init(const std::string &name, std::set<jackbergus::framework::FinestScaleTimeRepresentation> &time_arrow) {
        std::ifstream f{name};
        auto node = fkyaml::node::deserialize(f);
        const auto &struct_name = node["name"];
        auto &m = node["fields"].as_map();
        file_wrappers.resize(m.size());
        file_block_buffers.resize(m.size());
        field_names.resize(m.size());
        is_good.resize(m.size());
        field_offsets.resize(m.size());
        uint64_t i = 0;
        for (auto &[k, v]: node["fields"].as_map()) {
            const auto &field_name = k.as_str();

            {
                // Doing a first read, and getting all the shared temporal indices....
                jackbergus::framework::FileBlockReader<> tmp_file(v["binary"].as_str());
                uint64_t total = 0;
                while (tmp_file.read(tmp_buffer)) {
                    total += tmp_buffer.size();
                    for (uint64_t i = 0, N = tmp_buffer.size(); i < N; ++i) {
                        auto cp = tmp_buffer.get(i);
                        time_arrow.insert(cp.first->start);
                        time_arrow.insert(cp.first->end);
                    }
                }
            }

            // Now, preparing to actually read all the files
            auto &ref = file_wrappers[i];
            auto &buffer = file_block_buffers[i];
            ref.open(v["binary"].as_str()); // Opening the binary file for reading
            auto type = magic_enum::enum_cast<type_cases>(v["field_type"].as_str()).value();
            auto n_size = v["field_type_native_size"].as_int();
            auto k_name = v["field_name"].as_str();
            auto &field_back = fields.emplace_back(k_name, type, n_size);
            names_to_fieldsidx.emplace(k_name, fields.size() - 1);
            field_names[i] = struct_name.as_str() + "." + k_name;
            is_good[i] = ref.read(buffer);
            if (is_good[i]) {
                field_offsets[i].first = 0;
                field_offsets[i].second = buffer.size();
            } else {
                field_offsets[i].first = field_offsets[i].second = 0;
            }
            i++;
        }
    }

    std::pair<jackbergus::framework::BlockHeader *, void *> current(uint64_t column_idx) const {
        if ((column_idx >= is_good.size()) || (!is_good[column_idx])) {
            return {nullptr, nullptr};
        } else {
            auto &buffer = file_block_buffers[column_idx];
            return buffer.get(field_offsets[column_idx].first);
        }
    }

    bool next(uint64_t column_idx) {
        bool return_result = true;
        if (column_idx >= is_good.size()) {
            return_result = false;
        } else {
            if (is_good[column_idx]) {
                if (field_offsets[column_idx].second <= field_offsets[column_idx].first + 1) {
                    auto &ref = file_wrappers[column_idx];
                    auto &buffer = file_block_buffers[column_idx];
                    is_good[column_idx] = ref.read(buffer);
                    if (is_good[column_idx]) {
                        field_offsets[column_idx].first = 0;
                        field_offsets[column_idx].second = buffer.size();
                    } else {
                        field_offsets[column_idx].first = field_offsets[column_idx].second = 0;
                        return_result = false;
                    }
                } else {
                    field_offsets[column_idx].first++;
                }
            } else {
                return_result = false;
            }
        }
        return return_result;
    }

    // bool good() const {
    //     return is_good;
    // }

private:
    // bool is_good = true;
    std::vector<std::pair<uint64_t, uint64_t> > field_offsets;
    std::vector<bool> is_good;
    jackbergus::framework::FileBlockWrapper<> tmp_buffer;
    std::vector<jackbergus::framework::FileBlockReader<> > file_wrappers;
    std::vector<jackbergus::framework::FileBlockWrapper<> > file_block_buffers;
    std::vector<std::string> field_names;
    std::vector<FieldGetting> fields;
    std::unordered_map<std::string, uint64_t> names_to_fieldsidx;
};


// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    std::string file_name = "final_serialization_result.csv";
    std::random_device rd; // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<uint64_t> dista(10, 55); // Inclusive range

    {
        jackbergus::framework::AnyStructMonitoring<ExampleStructure> obj(0);
        obj.setFile("NitakungodeaMilele");
        ExampleStructure stru;
        for (uint64_t i = 0; i < 100; i++) {
            stru.a = dista(gen);
            std::cout << stru.a << std::endl;
            stru.b = dista(gen);
            stru.c = dista(gen);
            stru.d = dista(gen);
            stru.e = dista(gen);
            obj.updateValue(i, stru);
        }
        obj.clearFile();
    }


    jackbergus::framework::FileBlockReader<> flip("NitakungodeaMilele_a.bin");
    jackbergus::framework::FileBlockWrapper<> block;
    uint64_t total = 0;
    while (flip.read(block)) {
        total += block.size();
        for (uint64_t i = 0, N = block.size(); i < N; ++i) {
            auto cp = block.get(i);
            std::cout << *((uint64_t *) cp.second) << "@ [" << cp.first->start << ", " << cp.first->end << "]" <<
                    std::endl;
        }
    }
    std::vector<std::filesystem::path> paths_to_yamls;
    for (const auto &ref: std::filesystem::directory_iterator(
             "C:\\Users\\diste\\Downloads\\GenericFramework\\cmake-build-debug")) {
        if (ref.is_regular_file()) {
            // mimicking C++20 endswith
            auto str = ref.path().string();
            if (str.substr(str.size() - 5, 5) == ".yaml") {
                std::cout << str << std::endl;
                paths_to_yamls.emplace_back(ref.path());
            }
        }
    }

    std::set<jackbergus::framework::FinestScaleTimeRepresentation> time_arrow;
    std::vector<RecordFileDeserializer> yamls_to_readers;
    yamls_to_readers.resize(paths_to_yamls.size());
    for (uint64_t i = 0; i < paths_to_yamls.size(); i++) {
        auto &ref = yamls_to_readers[i];
        auto &yaml = paths_to_yamls[i];
        ref.init(yaml.filename(), time_arrow);
    }

    std::ofstream csv_file{file_name};
    for (const auto &ref: yamls_to_readers) {
        for (const auto &col_name: ref.columns()) {
            csv_file << "\"" << col_name << "\",";
        }
    }
    csv_file << "\"time\"" << std::endl;

    for (const auto &time_step: time_arrow) {
        for (auto &ref: yamls_to_readers) {
            for (uint64_t i = 0, N = ref.n_variables(); i < N; i++) {
                auto cp = ref.current(i);
                if ((!cp.first) || (!cp.second) || (cp.first->start_validity == 0) || (cp.first->end_validity == 0) || (
                        cp.first->start > time_step)) {
                    csv_file << "n/a,";
                } else {
                    bool performed_next = false;
                    if (time_step > cp.first->end) {
                        ref.next(i);
                        performed_next = true;
                    }
                    cp = ref.current(i);
                    if ((!cp.first) || (!cp.second) || (cp.first->start_validity == 0) || (cp.first->end_validity == 0)
                        || (cp.first->start > time_step)) {
                        csv_file << "n/a,";
                    } else {
                        if ((cp.first->start <= time_step) && (time_step <= cp.first->end)) {
                            const auto &info = ref.getFieldInfo(i);
                            switch (info->field_type) {
                                case T_SIGNED_INTEGRAL:
                                    if (info->field_value_size == 8) {
                                        csv_file << *(int64_t *) cp.second << ",";
                                    } else if (info->field_value_size == 4) {
                                        csv_file << *(int32_t *) cp.second << ",";
                                    } else if (info->field_value_size == 2) {
                                        csv_file << *(int16_t *) cp.second << ",";
                                    } else if (info->field_value_size == 1) {
                                        csv_file << *(int8_t *) cp.second << ",";
                                    } else {
                                        throw std::runtime_error("Invalid value size for signed integral");
                                    }
                                    break;

                                case T_U_INTEGRAL:
                                    if (info->field_value_size == 8) {
                                        csv_file << *(uint64_t *) cp.second << ",";
                                    } else if (info->field_value_size == 4) {
                                        csv_file << *(uint32_t *) cp.second << ",";
                                    } else if (info->field_value_size == 2) {
                                        csv_file << *(uint16_t *) cp.second << ",";
                                    } else if (info->field_value_size == 1) {
                                        csv_file << *(uint8_t *) cp.second << ",";
                                    } else {
                                        throw std::runtime_error("Invalid value size for unsigned integral");
                                    }
                                    break;

                                case T_SIGNED_FLOAT:
                                    if (info->field_value_size == 8) {
                                        csv_file << *(double *) cp.second << ",";
                                    } else if (info->field_value_size == 4) {
                                        csv_file << *(float *) cp.second << ",";
                                    } else {
                                        throw std::runtime_error("Invalid value size for signed float");
                                    }
                                    break;

                                case T_STRING: {
                                    csv_file << std::quoted(std::string((char *) cp.second, cp.first->payload_size)) <<
                                            ",";
                                    break;
                                }

                                case T_ENUM:
                                case T_U_FLOAT:
                                case T_STATIC_ARRAY:
                                case T_OTHER_ARRAY:
                                case T_UNION:
                                case T_CLASS:
                                case T_FUNCTION:
                                case T_POINTER:
                                case T_TUPLE:
                                case T_VARIANT:
                                case T_UNEXPECTED:
                                case T_VOID:
                                case T_NULLPTR:
                                    throw std::runtime_error("Invalid type");
                            }
                        }
                    }
                }
            }
        }
        csv_file << time_step << std::endl;
    }

    return 0;
}
