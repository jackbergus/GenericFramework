// QuickSort.h
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
// Created by Giacomo Bergami, PhD on 11/05/2026.
//

#ifndef GENERALFRAMEWORK_QUICKSORT_H
#define GENERALFRAMEWORK_QUICKSORT_H

#include <functional>
#include <cstdint>
#include <vector>
#include <limits>
#include <algorithm>

template <typename T, typename comp>
static inline uint64_t Partition(std::vector<T>& vec, uint64_t lo, uint64_t hi, const std::function<comp(const T&)>& f) {
    auto pivot = f(vec[hi]);
    uint64_t i = lo;
    for (uint64_t j = lo; j<hi; j++) {
        if ((f(vec[j]) < pivot) || (std::abs(f(vec[j]) - pivot)<=std::numeric_limits<comp>::epsilon())) {
            std::swap(vec[i],vec[j]);
            i++;
        }
    }
    std::swap(vec[hi],vec[i]);
    return i;
}

template <typename T, typename comp>
static inline  void QuickSort(std::vector<T>& vec, uint64_t lo, uint64_t hi, const std::function<comp(const T&)>& f) {
    if (lo >= hi) {
        return;
    } else {
        auto p = Partition(vec, lo, hi, f);
        if ((p> lo)) {
            QuickSort(vec, lo, p - 1, f);
        }
        QuickSort(vec, p + 1, hi, f);
    }
}

template <typename T, typename comp>
static inline void QuickSort(std::vector<T>& vec, const std::function<comp(const T&)>& f) {
    if ((vec.size() <= 1)) {
        return;
    } else {
        uint64_t lo = 0;
        uint64_t hi = vec.size() - 1;
        QuickSort(vec, lo, hi, f);
    }

}


#endif //GENERALFRAMEWORK_QUICKSORT_H
