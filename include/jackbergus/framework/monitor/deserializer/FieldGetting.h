// FieldGetting.h
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
// Created by Giacomo Bergami, PhD on 26/03/26.
//

#ifndef GENERALFRAMEWORK_FIELDGETTING_H
#define GENERALFRAMEWORK_FIELDGETTING_H

#include <cstdint>
#include <string>
#include <narcissus/reflection/type_cases.h>

namespace jackbergus {
    namespace data {
        namespace deserializer {
            struct FieldGetting {
                std::string field_name;
                type_cases field_type;
                uint64_t field_value_size;

                FieldGetting(const std::string &field_name, type_cases field_type, uint64_t field_value_size);
                FieldGetting();
                FieldGetting(const FieldGetting &) = default;
                FieldGetting(FieldGetting &&) = default;
                FieldGetting &operator=(const FieldGetting &) = default;
                FieldGetting &operator=(FieldGetting &&) = default;
            };
        } // serializer
    } // data
} // jackbergus

#endif //GENERALFRAMEWORK_FIELDGETTING_H