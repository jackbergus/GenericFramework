//
// Created by gyankos on 26/03/26.
//

#include <jackbergus/framework/monitor/deserializer/FieldGetting.h>

namespace jackbergus {
    namespace data {
        namespace deserializer {
            FieldGetting::FieldGetting(const std::string &field_name, type_cases field_type,
                uint64_t field_value_size): field_name(field_name),
                                            field_type(field_type),
                                            field_value_size(field_value_size) {
            }

            FieldGetting::FieldGetting(): field_name(""), field_type(T_UNEXPECTED), field_value_size{0} {
            }
        } // serializer
    } // data
} // jackbergus