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
                                    return false;
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
