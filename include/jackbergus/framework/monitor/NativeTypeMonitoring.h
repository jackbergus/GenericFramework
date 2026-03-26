//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_NATIVETYPEMONITORING_H
#define GENERALFRAMEWORK_NATIVETYPEMONITORING_H

#include <string>
#include <typeindex>

#include <narcissus/reflection/type_cases.h>
#include <cstdint>
#include <fstream>

namespace jackbergus {
    namespace framework {
        struct NativeTypeMonitoring {
            std::string name;           ///< Name of the field
            type_cases casusu;          ///< Flattened enum referring to the type
// #if __cplusplus >= 202302L
            std::type_index type_i;     ///< Type index
// #endif
            uint64_t idx, sizeof_;      ///< Position of the field within the structure, and allocated memory for representing the datum

            NativeTypeMonitoring(const NativeTypeMonitoring& ) = default;
            NativeTypeMonitoring(NativeTypeMonitoring&& ) = default;
            NativeTypeMonitoring& operator=(const NativeTypeMonitoring& ) = default;
            NativeTypeMonitoring& operator=(NativeTypeMonitoring&& ) = default;
            NativeTypeMonitoring(const std::string& name, type_cases casusu,
// #if __cplusplus >= 202302L
            const std::type_index& idx,
// #endif
            uint64_t record_offset, uint64_t sizeof_);

            template <typename T> bool write(const T& value, std::ofstream& file) {
// #if __cplusplus >= 202302L
                if (std::type_index(typeid(T)) != type_i)
                    return false;
// #endif
                file.write((const char*)&value, sizeof_);
                return file.good();
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_NATIVETYPEMONITORING_H