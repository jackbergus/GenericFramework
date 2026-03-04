//
// Created by gyankos on 04/03/26.
//

#include <jackbergus/framework/monitor/NativeTypeMonitoring.h>

namespace jackbergus {
    namespace framework {
        NativeTypeMonitoring::NativeTypeMonitoring(const std::string &name, type_cases casusu,
            const std::type_index &idx, uint64_t record_offset, uint64_t sizeof_): name(name), casusu(casusu), type_i(idx), idx{record_offset}, sizeof_(sizeof_) {}
    } // framework
} // jackbergus