// NativeTypeMonitoring.h
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

            void flush(std::ofstream& file) {
                file.flush();
            }

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