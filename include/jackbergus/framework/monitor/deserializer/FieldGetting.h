//
// Created by gyankos on 26/03/26.
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