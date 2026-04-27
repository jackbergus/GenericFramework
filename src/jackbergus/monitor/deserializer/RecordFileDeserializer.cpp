//
// Created by gyankos on 26/03/26.
//

#include "jackbergus/framework/monitor/deserializer//RecordFileDeserializer.h"

#include "fkYAML/node.hpp"
#include <magic_enum/magic_enum.hpp>

namespace jackbergus {
    namespace data {
        namespace deserializer {
            RecordFileDeserializer::RecordFileDeserializer() {
            }

            void RecordFileDeserializer::init(const std::string &name,
                std::set<jackbergus::framework::FinestScaleTimeRepresentation> &time_arrow) {
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
                        const auto& binary_file_name = v["binary"].as_str();
                        jackbergus::framework::FileBlockReader<> tmp_file(binary_file_name);
                        uint64_t total = 0;
                        framework::FileBlockWrapper<> tmp_buffer;
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

            std::pair<jackbergus::framework::BlockHeader *, void *> RecordFileDeserializer::current(
                uint64_t column_idx) const {
                if ((column_idx >= is_good.size()) || (!is_good[column_idx])) {
                    return {nullptr, nullptr};
                } else {
                    auto &buffer = file_block_buffers[column_idx];
                    return buffer.get(field_offsets[column_idx].first);
                }
            }

            bool RecordFileDeserializer::next(uint64_t column_idx) {
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
        } // deserializer
    } // data
} // jackbergus