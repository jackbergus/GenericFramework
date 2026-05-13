// ExternalMergeSort.h
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

#ifndef GENERALFRAMEWORK_EXTERNALMERGESORT_H
#define GENERALFRAMEWORK_EXTERNALMERGESORT_H

#include <jackbergus/framework/ndp/FileSerializer.h>
#include <jackbergus/data_structures/LRUCache.h>
#include <jackbergus/algorithms/QuickSort.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <list>
#include   <unordered_map>
#include <functional>
#include <jackbergus/framework/ndp/FileBlockReader.h>

template<uint64_t capacity>
struct FileLRUCache : public  LRUCache<capacity, std::string, std::FILE*> {

    std::FILE* getFile(const std::string& key) {
        std::FILE* result = nullptr;
        using super = LRUCache<capacity, std::string, std::FILE*>;
        auto it = super::cacheMap.find(key);
        if (it != super::cacheMap.end()) {
            result = it->second->second;
            super::moveToFront(key, result); // Update existing key
        } else {
            if (super::cacheList.size() == capacity) {
                auto last = super::cacheList.back();
                std::fclose(last.second);
                super::cacheMap.erase(last.first); // Remove least recently used item
                super::cacheList.pop_back();
            }
            result = std::fopen(key.c_str(), "rb");
            super::cacheList.push_front({key, result}); // Add new item to the front
            super::cacheMap[key] = super::cacheList.begin();
        }
        return result;
    }

    bool close(const std::string& key) {
        using super = LRUCache<capacity, std::string, std::FILE*>;
        auto it = super::cacheMap.find(key);
        if (it == super::cacheMap.end()) {
            return false;
        } else {
            auto fptr = it->second->second;
            super::cacheList.erase(it->second);
            super::cacheMap.erase(it);
            std::fclose(fptr);
            return true;
        }
    }

};

struct MinHeapIovec {
    jackbergus::framework::new_delta_data_structure data;
    uint64_t  filename_idx{0};

    MinHeapIovec() {}
    MinHeapIovec(const jackbergus::framework::new_delta_data_structure& data,
        uint64_t idx) : data{data}, filename_idx(idx) {}

    friend bool operator<(const MinHeapIovec &lhs, const MinHeapIovec &rhs) {
        return lhs.data.timestamp > rhs.data.timestamp ||
            ( lhs.data.timestamp == rhs.data.timestamp && lhs.data.structure_id > rhs.data.structure_id)  ||
            (lhs.data.timestamp == rhs.data.timestamp && lhs.data.structure_id == rhs.data.structure_id &&  lhs.data.unnested_field_id > rhs.data.unnested_field_id);
    }

    friend bool operator<=(const MinHeapIovec &lhs, const MinHeapIovec &rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>(const MinHeapIovec &lhs, const MinHeapIovec &rhs) {
        return rhs < lhs;
    }

    friend bool operator>=(const MinHeapIovec &lhs, const MinHeapIovec &rhs) {
        return !(lhs < rhs);
    }
};



template<uint64_t maximum_slot_size = 3,
         uint64_t max_fd_opened = 2>
struct ExternalMergeSort {

    ExternalMergeSort(const std::string& binary_filename) : binary_file{binary_filename} {

    };

    void start() {
        // Starting with the sorting procedure
        std::filesystem::path p;
        std::vector<std::string> serialized_vector;

        // 1) Opening the binary file for reading.
        uint64_t counting = 0;
        uint64_t number_of_serialized_files = 0;
        std::vector<jackbergus::framework::new_delta_data_structure> tmp_data;
        fbr.open(binary_file);

        constexpr uint64_t RECORD_SIZE = sizeof(jackbergus::framework::new_delta_data_structure);

        // Now, computing the external merge sort using a min-heap, thus assuming that each single record of the file can fit in
        // primary memory. The assumption here is that not all file descriptors can be opened at the same time, thus
        // I am keeping a fixed amount of files opened, forsooth.
        // This part of the code was originally from Giacomo Bergami's VarSorter project, forsooth!
        // https://github.com/jackbergus/varsorter/blob/legacy/external_merge_sort/external_merge_sort.h
        /* * Copyright (C) 2019 - Giacomo Bergami
        *
        * varsorter is free software; you can redistribute it and/or modify
        * it under the terms of the GNU General Public License as published by
        * the Free Software Foundation; either version 2 of the License, or
        * any later version.
        *
        * varsorter is distributed in the hope that it will be useful,
        * but WITHOUT ANY WARRANTY; without even the implied warranty of
        * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        * GNU General Public License for more details.
        *
        * You should have received a copy of the GNU General Public License
        * along with varsorter. If not, see <http://www.gnu.org/licenses/>.*/

        // Create a min heap with k heap nodes (miniheap_iovec).  Every heap node
        // has first element of scratch output file
        std::priority_queue<MinHeapIovec, std::vector<MinHeapIovec>> minheap;
        std::vector<uint64_t> ptrs, ends;

        auto F = [](const jackbergus::framework::new_delta_data_structure& x ) { return std::tie(x.timestamp, x.structure_id, x.unnested_field_id); };
        while (fbr.read(buffer, false)) {
            for (uint64_t i = 0, N = buffer.size(); i < N; i++) {
                auto ptr = buffer.getNewRecord(i);
                tmp_data.emplace_back(*ptr);
                counting++;
                if (counting == maximum_slot_size && counting>0) {
                    if (number_of_serialized_files == 0) {
                        // creating a folder based on the current name
                        p = binary_file+"_dir";
                        std::filesystem::create_directories(p);
                    }

                    // While we're at it, without using multithreading, sorting the element being stored in temporary memory
                    QuickSort<jackbergus::framework::new_delta_data_structure, std::tuple<double, uint8_t, uint8_t>>(tmp_data, F);
                    auto new_file = p / (std::to_string(number_of_serialized_files)+".frag");
                    {
                        std::ofstream file{new_file, std::ios_base::binary | std::ios_base::out};
                        for (const auto& ref : tmp_data) {
                            file.write(reinterpret_cast<const char*>(&ref), sizeof(ref));
                        }
                    }
                    minheap.emplace(tmp_data[0], number_of_serialized_files);
                    serialized_vector.emplace_back(new_file.string());
                    ptrs.emplace_back(0);
                    ends.emplace_back(tmp_data.size());
                    tmp_data.clear();
                    number_of_serialized_files++;\
                    counting = 0;
                }
            }
        }
        if (counting > 0) {
            QuickSort<jackbergus::framework::new_delta_data_structure, std::tuple<double, uint8_t, uint8_t>>(tmp_data, F);
            
            if (number_of_serialized_files> 0) {
                        auto new_file = p / (std::to_string(number_of_serialized_files)+".frag");
            {
                std::ofstream file{new_file, std::ios_base::binary | std::ios_base::out};
                for (const auto& ref : tmp_data) {
                    file.write(reinterpret_cast<const char*>(&ref), sizeof(ref));
                }
            }
            minheap.emplace(tmp_data[0], number_of_serialized_files);
            serialized_vector.emplace_back(new_file.string());
            ptrs.emplace_back(0);
            ends.emplace_back(tmp_data.size());
            tmp_data.clear();
            number_of_serialized_files++;
            counting = 0;
            } else {
            	// If I was able to sort this entire file in primary memory and to re-sort it back in a serialized way, then 
            	fbr.close();
            	jackbergus::framework::FileSerializer<> file{binary_file};
            	for (const auto& data : tmp_data)
            		file.write((void*)(&data), sizeof(data));
            	file.flush();
            	file.close();
            	counting = tmp_data.size();
            	number_of_serialized_files = 1;
            	return; // Quitting, as I have already written everything, forsooth!
            }
        }


        std::string bin_sorted = binary_file+".sorted";
        {
            FileLRUCache<max_fd_opened> opened_files;
            {
                jackbergus::framework::FileSerializer<> file{bin_sorted};
                //std::ofstream file{bin_sorted, std::ios_base::binary | std::ios_base::out};
                int k = serialized_vector.size();
                while (!minheap.empty()) {
                    const struct MinHeapIovec& x = minheap.top();
                    uint64_t idx = x.filename_idx;
                    std::cout << " SERIALIZE: " << magic_enum::enum_name(x.data.actual_type).data() << " for " << (uint64_t)x.data.structure_id << " of " << (uint64_t)x.data.unnested_field_id << " @ " << x.data.timestamp << std::endl;
                    file.write((void*)(&x.data), sizeof(x.data));

                    if (ptrs[idx] != ends[idx]) {
                        ptrs[idx]++;
                    }
                    minheap.pop();
                    if (ptrs[idx] != ends[idx]) {
                        auto fptr = opened_files.getFile(serialized_vector[idx].c_str());
                        std::fseek(fptr, ptrs[idx] * RECORD_SIZE, SEEK_SET);
                        MinHeapIovec buffer;
                        buffer.filename_idx = idx;
                        std::fread(&buffer.data, sizeof(buffer.data), 1, fptr);
                        minheap.push(std::move(buffer));
                    } else {
                        opened_files.close(serialized_vector[idx].c_str());
                        std::remove(serialized_vector[idx].c_str());
                        //std::cout << "Closing "<< serialized_vector[idx] << " @" << idx << std::endl;
                    }
                }
                file.flush();
                file.close();
            }
        }
        std::filesystem::remove_all(p);
        std::remove(binary_file.c_str());
        std::rename(bin_sorted.c_str(), binary_file.c_str());
    }

private:
    //
    // Reading the binary file, as per previous serialization
    jackbergus::framework::FileBlockReader<> fbr;
    // Buffer containing one page of memory per entry
    jackbergus::framework::FileBlockWrapper<> buffer;
    std::string binary_file;
};

#endif //GENERALFRAMEWORK_EXTERNALMERGESORT_H
