// LRUCache.h
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

#ifndef GENERALFRAMEWORK_LRUCACHE_H
#define GENERALFRAMEWORK_LRUCACHE_H

#include <cstdint>
#include <list>
#include <unordered_map>

template<uint64_t capacity, typename Key, typename Value>
struct  LRUCache {
    void moveToFront(const Key& key, const Value& value) {
        cacheList.erase(cacheMap[key]); // Remove the old position
        cacheList.push_front({key, value}); // Add to the front
        cacheMap[key] = cacheList.begin(); // Update the map
    }

    std::list<std::pair<Key, Value>> cacheList; // Doubly linked list to store keys and values
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> cacheMap; // Hash map for O(1) access

    Value* get(const Key& key) {
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            return nullptr; // Key not found
        }
        moveToFront(key, it->second); // Update usage
        return &cacheMap.find(key)->second->second;
    }

    void put(const Key& key, const Value& value) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            moveToFront(key, value); // Update existing key
        } else {
            if (cacheList.size() == capacity) {
                auto last = cacheList.back();
                cacheMap.erase(last.first); // Remove least recently used item
                cacheList.pop_back();
            }
            cacheList.push_front({key, value}); // Add new item to the front
            cacheMap[key] = cacheList.begin();
        }
    }
};

#endif //GENERALFRAMEWORK_LRUCACHE_H
