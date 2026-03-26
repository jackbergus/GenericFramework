//
// Created by gyankos on 04/03/26.
//

#include <jackbergus/framework/monitor/NativeTypeMonitoring.h>

namespace jackbergus {
    namespace framework {
        NativeTypeMonitoring::NativeTypeMonitoring(const std::string &name, type_cases casusu,
// #if __cplusplus >= 202302L
            const std::type_index &idx,
// #endif
            uint64_t record_offset, uint64_t sizeof_): name(name), casusu(casusu),
// #if __cplusplus >= 202302L
        type_i(idx),
// #endif
        idx{record_offset}, sizeof_(sizeof_) {}
    } // framework
} // jackbergus