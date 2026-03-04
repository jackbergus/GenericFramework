// FileSerializer.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - gyankos
//
// narcissus is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  narcissus is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with narcissus. If not, see <http://www.gnu.org/licenses/>.

#ifndef GENERALFRAMEWORK_FILESERIALIZER_H
#define GENERALFRAMEWORK_FILESERIALIZER_H
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <cstring>

#include <jackbergus/framework/types/NativeTypes.h>

namespace jackbergus {
    namespace framework {
        struct BlockHeader {
            FinestScaleTimeRepreentation start;
            FinestScaleTimeRepreentation end;
            uint8_t                      start_validity ;
            uint8_t                      end_validity ;
            char                         logger_record[126];
            uint64_t                     payload_size;
        };

        /**
         * File Serialization in a block-wise structure, so to better read contiguously the file by block size without any requirement for memory mapping or the like.
         * @tparam block_size Size of the fixed-size block containing all the records of potential variable size. This is to improve over data reading in a forthcoming step
         */
        template<uint64_t block_size = 1024>
        class FileSerializer {
            constexpr static uint64_t BUFFER_SIZE = block_size-sizeof(uint64_t);
            char buffer[BUFFER_SIZE];
            uint64_t written_element;
            uint64_t written_offset;
            std::ofstream stream;
            bool stream_is_open;
            std::string file_name;

        public:



            uint64_t getFileNameLen() const {
                return (!stream_is_open) ? 0 : file_name.size();
            }

            const char* getCFileName() const {
                return (!stream_is_open) ? nullptr : file_name.c_str();
            }

            const std::string getFileName(uint64_t max_len = std::numeric_limits<uint64_t>::max()) const {
                if (!stream_is_open)
                    return "";
                else {
                    if (max_len == std::numeric_limits<uint64_t>::max()) {
                        return file_name;
                    } else {
                        return file_name.substr(0, std::min(max_len, file_name.size())-1);
                    }
                }
            }

            FileSerializer(const std::string& filename) : stream(filename, std::ios::binary), file_name(filename) {
                stream_is_open = std::filesystem::is_regular_file(filename) || (!std::filesystem::is_directory(filename));
                written_element = 0;
                written_offset = 0;
                static_assert(sizeof(BlockHeader) < block_size, "BlockHeader size mismatch");
            }

            template <typename T>
            bool write(const BlockHeader& block, const T& obj) {
                return write(block, (void*)(&obj), sizeof(T));
            }

            bool write(const BlockHeader& block, void* data_ptr, uint64_t data_size) {
                if (!stream_is_open) return false;
                else {
                    if (data_size + sizeof(BlockHeader) > BUFFER_SIZE) {
                        return false; // ERROR: cannot have the data and the block to be greater than the actual block size. Something fishy is going here...
                    } else {
                        if (data_size + sizeof(BlockHeader) + written_offset > BUFFER_SIZE) {
                            flush();
                        }
                        memcpy(&buffer[written_offset], (void*)&block, sizeof(block));
                        memcpy(&buffer[written_offset+sizeof(block)], data_ptr, data_size);
                        written_offset += (data_size + sizeof(block));
                        written_element += 1;
                        return true;
                    }
                }
            }

            void close() {
                if (stream_is_open) {
                    flush();
                    stream.close();
                    stream_is_open = false;
                } else {
                    // noop
                }
            }

            void flush() {
                // Forces to write a block to disk
                if (stream_is_open) {
                    stream.write(reinterpret_cast<char*>(&written_element), sizeof(uint64_t));
                    stream.write(reinterpret_cast<char*>(&buffer), BUFFER_SIZE);
                    written_element = 0;
                    written_offset = 0;
                } else {
                    // noop
                }
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_FILESERIALIZER_H
