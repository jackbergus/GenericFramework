// binary_to_csv_serializer.cpp
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
// Created by Giacomo Bergami, PhD on 29/04/2026.
//

#include <fkYAML/node.hpp>
#include <magic_enum/magic_enum.hpp>
#include <fstream>
#include <map>
#include <unordered_map>
#include <jackbergus/framework/monitor/serializer/binary_to_csv_serializer.h>
#include <jackbergus/framework/monitor/deserializer/FieldGetting.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>

void sort_and_convert_binary_to_csv(const std::string &binary_file,
                                    const std::string &final_csv_file,
                                    std::vector<std::string> &admissible_headers_for_serialization) {
    {
        ExternalMergeSort<> test(binary_file);
        test.start()   ;
    }
    convert_binary_to_csv(binary_file, final_csv_file, admissible_headers_for_serialization);
}

void convert_binary_to_csv(const std::string &binary_file,
                           const std::string &final_csv_file,
                           std::vector<std::string> &admissible_headers_for_serialization) {
    // Fully qualified field name to offset as a column within the header file
    std::unordered_map<std::string, uint64_t> toConsiderFields_to_header_offset;
    // Mapping binary struct-id and field-id to the column offset
    std::map<std::pair<uint8_t, uint8_t>, uint64_t> binary_pair_info_to_admissible_headers_for_serialization_offset;
    // If admissible_headers_for_serialization is empty, then I cam considering all the fields within the binary by default
    bool considerAllFields = admissible_headers_for_serialization.empty();
    // For determining whether the file was correctly serialized, mapping each column offset id to the structure id, so to determine whether all the fields serialized for the current iteration were considered
    std::vector<uint8_t> col_offset_to_struct_id;
    if (!considerAllFields) {
        for (uint64_t idx = 0, N = admissible_headers_for_serialization.size(); idx < N; ++idx) {
            toConsiderFields_to_header_offset[admissible_headers_for_serialization[idx]] = idx;
        }
        col_offset_to_struct_id.resize(admissible_headers_for_serialization.size(), 0);
    }
    // Result of the deserialization of the yaml file, as this contains the information of the structure itself as well as of the fields within it
    std::vector<jackbergus::data::deserializer::SerializedStructureInfo> yaml_result;
    // Mapping the structure name to the offset within yaml_result where such name is present
    std::unordered_map<std::string, uint64_t> struct_name_to_vector_offset;
    // Temporary variable mapping the structure id to the field id. USed to efficiently accessing maps having this kind of data representation as a key
    std::pair<uint8_t, uint8_t> binary_coordinates;
    {
        auto yaml_file = binary_file + ".yaml";
        uint64_t colN = 0;
        std::ifstream f{yaml_file};
        auto root = fkyaml::node::deserialize(f);
        std::map<uint64_t, jackbergus::data::deserializer::SerializedStructureInfo> yaml_content;
        // As the elements within the yaml file might appear in an order being different from the one of the actual index (as the string-based map is not serialized by order of insertion), I am keeping the info of the order via the indices here

        const auto &binary_name = root["binary"];
        auto &m = root["structs"].as_map();

        // Mainly, filling in the data structures
        for (auto &[struct_name, node]: root["structs"].as_map()) {
            binary_coordinates.first = node["index"].as_int();
            auto &actual_content = yaml_content[binary_coordinates.first];
            actual_content.struct_id = binary_coordinates.first;
            actual_content.struct_name = struct_name.as_str();
            struct_name_to_vector_offset[actual_content.struct_name] = binary_coordinates.first;
            std::string name = node["name"].as_str();
            auto &m = node["fields"].as_map();
            std::map<uint64_t, jackbergus::data::deserializer::FieldGetting> fields;
            uint64_t i = 0;
            for (auto &[k, v]: node["fields"].as_map()) {
                const auto &field_name = k.as_str();
                binary_coordinates.second = (uint64_t) v["field_idx"].as_int();
                auto &target = fields[binary_coordinates.second];
                target.field_type = magic_enum::enum_cast<type_cases>(v["field_type"].as_str()).value();
                target.field_value_size = v["field_type_native_size"].as_int();
                target.field_name = v["field_name"].as_str();
                auto compact_name = actual_content.struct_name + "." + target.field_name;
                if (considerAllFields) {
                    auto res = admissible_headers_for_serialization.size();
                    binary_pair_info_to_admissible_headers_for_serialization_offset[binary_coordinates] = res;
                    admissible_headers_for_serialization.emplace_back(compact_name);
                    col_offset_to_struct_id.emplace_back(binary_coordinates.first);
                } else {
                    auto it = toConsiderFields_to_header_offset.find(compact_name);
                    if (it != toConsiderFields_to_header_offset.end()) {
                        binary_pair_info_to_admissible_headers_for_serialization_offset[binary_coordinates] = it->
                                second;
                        col_offset_to_struct_id[it->second] = binary_coordinates.first;
                    }
                }
                if (target.field_type == type_cases::T_ENUM) {
                    // Recovering the enum value, thus I am attempting at serializing a potential enum with the actual associated names.
                    for (auto &[idx, name]: v["field_enum_info"].as_map()) {
                        target.field_value_map[idx.as_int()] = name.as_str();
                    }
                }
                i++;
            }
            // After this is done, by assuming that all the indices are dense and always start form zero, moving the information in key order so to populate a vector.
            for (auto &[idx, result]: fields) {
                auto &moved = actual_content.fields.emplace_back(std::move(result));
                actual_content.field_names.emplace_back(moved.field_name);
                actual_content.names_to_fieldsidx[moved.field_name] = idx;
            }
        }
        // Same as for the fileds, but for the data structures, moving those in an orderly fashion, so that they can be accessed through a vector in a more convenient way.
        for (auto &[_, rec]: yaml_content) {
            yaml_result.emplace_back(std::move(rec));
        }
    }

    // Opening the single binary file containing the info for all the data structures in an orderly fashion
    jackbergus::framework::FileBlockReader<> fbr;
    // Buffer containing one page of memory per entry
    jackbergus::framework::FileBlockWrapper<> buffer;
    fbr.open(binary_file);

    // Preparing to serialize the binary into a human-readable format in CSV
    std::ofstream csv_file{final_csv_file};
    uint64_t colN = admissible_headers_for_serialization.size();
    // Serializing the header
    for (const auto &col_name: admissible_headers_for_serialization) {
        csv_file << "\"" << col_name << "\",";
    }
    csv_file << "\"time\"" << std::endl;
    // By default, the cells are not set, thus they are n/a
    std::vector<std::string> previous_string_values(colN, "n/a");
    // When updating a structure, determinig whether this was started to be written, and whether such starting ended at some point.
    // This is relevant to determine if something went wrong with the serialization
    std::vector<std::pair<bool, bool> > is_record_fully_serialized_at_current_time(yaml_result.size(), {false, false});

    std::stringstream ss;
    ss.precision(64);
    csv_file.precision(64);

    double currentTime = 0.0;
    //Assuming by design that all first records are initialized with the default values at time zero, and that the rest will follow

    while (fbr.read(buffer, false)) {
        for (uint64_t i = 0, N = buffer.size(); i < N; i++) {
            auto ptr = buffer.getNewRecord(i);
            if (ptr->timestamp != currentTime) {
                // Actual syntactical equivalence. This is the same value that I got from the file. Thus, I am not requiring to have epsilon equivalence, forsooth!
                for (uint64_t col_offset = 0, M = previous_string_values.size(); col_offset < M; col_offset++) {
                    auto &val = previous_string_values[col_offset];
                    auto col_id = col_offset_to_struct_id[col_offset];
                    if ((!is_record_fully_serialized_at_current_time[col_id].first) || (
                            is_record_fully_serialized_at_current_time[col_id].second))
                        csv_file << val << ",";
                    else {
                        // If there was something wrong, and the current data structure was not fully written in this instant of time, then writing some n/a values, to represent the broken data!
                        val = "n/a";
                        csv_file << val << ",";
                    }
                }
                // Finally, writing the current timestamp
                csv_file << currentTime << std::endl;
                // Progressing with the other element
                currentTime = ptr->timestamp;
                // Clearing the validity bits
                for (uint64_t j = 0, M = yaml_result.size(); j < M; j++) {
                    is_record_fully_serialized_at_current_time[j] = {false, false};
                }
            }
            binary_coordinates.first = ptr->structure_id;
            binary_coordinates.second = ptr->unnested_field_id;
            const auto &structure = yaml_result.at(ptr->structure_id);

            auto it = binary_pair_info_to_admissible_headers_for_serialization_offset.find(binary_coordinates);

            if (it == binary_pair_info_to_admissible_headers_for_serialization_offset.end()) {
                continue; // That is, this value is not registered to be serialized in the final csv file
            }
            if (ptr->is_starting_of_structure)
                is_record_fully_serialized_at_current_time[binary_coordinates.first].first = true;
            if (ptr->is_end_of_structure)
                is_record_fully_serialized_at_current_time[binary_coordinates.first].second = true;
            const auto info = &structure.fields.at(ptr->unnested_field_id);
            std::string &previous_string_value = previous_string_values[it->second];
            // Getting the previous value that was set for this. And, according to this, printing the cell value and storing this into a vector of string for persistency, and for linearizing the write once a new timestamp occurs
            switch (info->field_type) {
                case T_SIGNED_INTEGRAL:
                    if (info->field_value_size == 8) {
                        previous_string_value = std::to_string(*(int64_t *) &ptr->actual_data);
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 4) {
                        previous_string_value = std::to_string(*(int32_t *) &ptr->actual_data);
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 2) {
                        previous_string_value = std::to_string(*(int16_t *) &ptr->actual_data);
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 1) {
                        previous_string_value = std::to_string(static_cast<int>(*(int8_t *) &ptr->actual_data));
                        //csv_file << previous_string_value << ",";
                    } else {
                        throw std::runtime_error("Invalid value size for signed integral");
                    }
                    break;

                case T_U_INTEGRAL:
                    if (info->field_value_size == 8) {
                        previous_string_value = std::to_string(*(uint64_t *) &ptr->actual_data);
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 4) {
                        previous_string_value = std::to_string(*(uint32_t *) &ptr->actual_data);
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 2) {
                        previous_string_value = std::to_string(*(uint16_t *) &ptr->actual_data);
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 1) {
                        previous_string_value = std::to_string(static_cast<int>(*(uint8_t *) &ptr->actual_data));
                        //csv_file << previous_string_value << ",";
                    } else {
                        throw std::runtime_error("Invalid value size for unsigned integral");
                    }
                    break;

                case T_SIGNED_FLOAT:
                    if (info->field_value_size == 8) {
                        ss.str(std::string());
                        ss << *(double *) &ptr->actual_data;
                        previous_string_value = ss.str();
                        //csv_file << previous_string_value << ",";
                    } else if (info->field_value_size == 4) {
                        ss.str(std::string());
                        ss << *(float *) &ptr->actual_data;
                        previous_string_value = ss.str();
                        //csv_file << previous_string_value << ",";
                    } else {
                        throw std::runtime_error("Invalid value size for signed float");
                    }
                    break;

                case T_ENUM: {
                    auto it2 = info->field_value_map.find(ptr->actual_data);
                    if (it2 != info->field_value_map.end()) {
                        previous_string_value = "\"" + it2->second + "\"";
                    } else {
                        previous_string_value = "NaN";
                    }
                    break;
                }

                case T_STRING:
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
                    continue;
            }
        }
    }
    for (const auto &val: previous_string_values) {
        csv_file << val << ",";
    }
    csv_file << currentTime << std::endl;
}
