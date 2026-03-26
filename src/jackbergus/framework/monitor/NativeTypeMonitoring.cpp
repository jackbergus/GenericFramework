// NativeTypeMonitoring.cpp
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