//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_FILEBLOCKWRAPPER_H
#define GENERALFRAMEWORK_FILEBLOCKWRAPPER_H

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <utility>
#include <vector>

#include <jackbergus/framework/ndp/FileSerializer.h>

namespace jackbergus {
    namespace framework {
/**
 * This class provides a logical wrapper of a block on disk, as well as constituiting a buffer to aforementioned file
 * @tparam block_size Size of a block on disk
 */
template <uint64_t block_size = 1024>
struct FileBlockWrapper {
    static constexpr uint64_t BLOCK_SIZE = block_size;

    FileBlockWrapper() : valid(false) {
        clear();
    }
    FileBlockWrapper(FileBlockWrapper<block_size>&& val) noexcept : index(std::move(val.index)) {
        if (val.valid) {
            memcpy(buffer, val.buffer, block_size*sizeof(char));
            valid = true;
        } else {
            valid = false;
        }
    }
    FileBlockWrapper(const FileBlockWrapper<block_size>& val) : index(val.index) {
        if (val.valid) {
            memcpy(buffer, val.buffer, block_size*sizeof(char));
            valid = true;
        } else {
            valid = false;
        }
    }
    FileBlockWrapper(FILE* fptr) {
        next(fptr);
    }

    /**
     * Reading the next block and clearing the current one.
     * @param fptr File pointer from which read the data
     */
    void next(FILE* fptr) {
        int bytes = 0;
        clear();
        if ((bytes = fread(buffer, sizeof(char), 1024, fptr)) == block_size) {
            valid = true;
            _index();
        } else {
            valid = false;
            clear();
        }
    }

    /**
     * Zeroizing the memory
     */
    void clear() {
        memset(buffer, 0, sizeof(char)*block_size);
    }

    /**
     *
     * @return If the file is valid, returns the number of records contained within the record
     */
    const uint64_t size() const {
        return valid ? *(uint64_t *) buffer : 0;
    }

    bool isValidInput() const {
        return valid;
    }

    std::pair<jackbergus::framework::BlockHeader*, void*> get(uint64_t idx) const {
        auto N = size();
        if (idx < size()) {
            jackbergus::framework::BlockHeader* first_header = (jackbergus::framework::BlockHeader*)(( buffer)+index[idx]);
            void* payload = (((char*)first_header)+sizeof(jackbergus::framework::BlockHeader));
            return {first_header, payload};
        } else {
            return {nullptr, nullptr};
        }
    }


private:
    void _index() {
        auto N = size();
        if (index.empty()) {
            jackbergus::framework::BlockHeader* first_header = (jackbergus::framework::BlockHeader*)(((uint64_t *) buffer)+1);
            for (uint64_t i = 0; i < N; ++i) {
                ptrdiff_t val = ((char*)first_header)-((char*)buffer);
                index.emplace_back(val);
                auto address = ( jackbergus::framework::BlockHeader*)(buffer+val);
                assert(address == first_header);
                // double val = *(double*)(((char*)first_header)+sizeof(jackbergus::framework::BlockHeader));
                // std::cout << val << "@ [" << first_header->start << ", " << first_header->end << "]" << std::endl;
                first_header = (jackbergus::framework::BlockHeader*)(((char*)first_header)+sizeof(jackbergus::framework::BlockHeader)+first_header->payload_size);
            }
        }
    }

    std::vector<uint64_t> index;
    bool valid;
    char buffer[block_size];
};
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_FILEBLOCKWRAPPER_H