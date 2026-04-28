//
// Created by gyankos on 26/03/26.
//

#include "jackbergus/framework/monitor//serializer/binary_to_csv_serializer.h"

#include <filesystem>
#include <set>
#include <vector>

#include "jackbergus/framework/monitor//deserializer/RecordFileDeserializer.h"
#include "jackbergus/framework/types/NativeTypes.h"

bool binary_to_csv_serializer(const std::string& folder,
                              const std::string& file_name) {

    if (!std::filesystem::is_directory(folder))
        return false;

    if (std::filesystem::is_directory(file_name))
        return false;

    std::vector<std::filesystem::path> paths_to_yamls;
    for (const auto &ref: std::filesystem::directory_iterator(
             folder)) {
        if (ref.is_regular_file()) {
            // mimicking C++20 endswith
            auto str = ref.path().string();
            if (str.substr(str.size() - 5, 5) == ".yaml") {
                // std::cout << str << std::endl;
                paths_to_yamls.emplace_back(ref.path());
            }
        }
    }

    std::set<jackbergus::framework::FinestScaleTimeRepresentation> time_arrow;
    std::vector<jackbergus::data::deserializer::RecordFileDeserializer> yamls_to_readers;
    std::vector<std::pair<jackbergus::framework::BlockHeader*, jackbergus::framework::BlockHeader*>> position_vector;
    yamls_to_readers.resize(paths_to_yamls.size());

    for (uint64_t i = 0; i < paths_to_yamls.size(); i++) {
        auto &ref = yamls_to_readers[i];
        auto &yaml = paths_to_yamls[i];
        ref.init(yaml.filename().string(), time_arrow);
    }

    std::ofstream csv_file{file_name};
    uint64_t colN = 0;
    for (const auto &ref: yamls_to_readers) {
        for (const auto &col_name: ref.columns()) {
            colN++;
            csv_file << "\"" << col_name << "\",";
            position_vector.emplace_back(nullptr, nullptr);
        }
    }
    csv_file << "\"time\"" << std::endl;
    std::vector<std::string> previous_string_values(colN, "n/a");
    std::stringstream ss;
    ss.precision(64);
    csv_file.precision(64);
    const auto N = yamls_to_readers.size();
    for (const auto &time_step: time_arrow) {
        uint64_t colID = 0;
        for (auto& ref : yamls_to_readers) {
            for (uint64_t i = 0, N = ref.n_variables(); i < N; i++) {
                std::string& previous_string_value = previous_string_values[colID];
                auto prev_curr = position_vector[colID];
                auto cp = ref.current(i);
                if (prev_curr.first == nullptr) {
                    prev_curr.first = cp.first;
                }

                if ((!cp.first) || (!cp.second) || (cp.first->start_validity == 0) || (cp.first->end_validity == 0) || (prev_curr.second && prev_curr.second->end != prev_curr.first->start && (prev_curr.second->end < time_step) && (time_step < prev_curr.first->start))) {
                    previous_string_value = "n/a";
                    csv_file << previous_string_value << ",";
                } else if ((cp.first->start > time_step)) {
                    // Keeping the previous value
                    csv_file << previous_string_value << ",";
                } else {
                    bool performed_next = false;
                    if (time_step > cp.first->end) {
                        ref.next(i);
                        performed_next = true;
                    }
                    cp = ref.current(i);
                    prev_curr.second = prev_curr.first;
                    prev_curr.first = cp.first;
                    if ((!cp.first) || (!cp.second) || (cp.first->start_validity == 0) || (cp.first->end_validity == 0) || (prev_curr.second && prev_curr.second->end != prev_curr.first->start && (prev_curr.second->end < time_step) && (time_step < prev_curr.first->start))
                        ) {
                        previous_string_value = "n/a";
                        csv_file << previous_string_value << ",";
                    } else if ((cp.first->start > time_step)) {
                        csv_file << previous_string_value << ",";
                    }
                    else {
                        if ((cp.first->start <= time_step) && (time_step <= cp.first->end)) {
                            const auto &info = ref.getFieldInfo(i);
                            switch (info->field_type) {
                                case T_SIGNED_INTEGRAL:
                                    if (info->field_value_size == 8) {
                                        previous_string_value = std::to_string(*(int64_t *) cp.second);
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 4) {
                                        previous_string_value = std::to_string(*(int32_t *) cp.second);
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 2) {
                                        previous_string_value = std::to_string(*(int16_t *) cp.second);
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 1) {
                                        previous_string_value = std::to_string(static_cast<int>(*(int8_t *) cp.second));
                                        csv_file << previous_string_value << ",";
                                    } else {
                                        throw std::runtime_error("Invalid value size for signed integral");
                                    }
                                    break;

                                case T_U_INTEGRAL:
                                    if (info->field_value_size == 8) {
                                        previous_string_value = std::to_string(*(uint64_t *) cp.second);
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 4) {
                                        previous_string_value = std::to_string(*(uint32_t *) cp.second);
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 2) {
                                        previous_string_value = std::to_string(*(uint16_t *) cp.second);
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 1) {
                                        previous_string_value = std::to_string(static_cast<int>(*(uint8_t *) cp.second));
                                        csv_file << previous_string_value << ",";
                                    } else {
                                        throw std::runtime_error("Invalid value size for unsigned integral");
                                    }
                                    break;

                                case T_SIGNED_FLOAT:
                                    if (info->field_value_size == 8) {
                                        ss.str(std::string());
                                        ss << *(double *) cp.second;
                                        previous_string_value = ss.str();
                                        csv_file << previous_string_value << ",";
                                    } else if (info->field_value_size == 4) {
                                        ss.str(std::string());
                                        ss << *(double *) cp.second;
                                        previous_string_value = ss.str();
                                        csv_file << previous_string_value << ",";
                                    } else {
                                        throw std::runtime_error("Invalid value size for signed float");
                                    }
                                    break;

                                case T_STRING: {
                                    ss.str(std::string());
                                    ss << std::quoted(std::string((char *) cp.second, cp.first->payload_size));
                                    previous_string_value = ss.str();
                                    csv_file << previous_string_value <<  ",";
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
                                    return false;
                            }
                        }
                    }
                }
                colID++;
            }
        }
        csv_file << time_step << std::endl;
    }

    return 0;

}
