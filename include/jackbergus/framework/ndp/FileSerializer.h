// FileSerializer.h
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

#ifndef GENERALFRAMEWORK_FILESERIALIZER_H
#define GENERALFRAMEWORK_FILESERIALIZER_H
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <cstring>

#include <narcissus/reflection/type_cases.h>
#include <jackbergus/framework/types/NativeTypes.h>

#include <magic_enum/magic_enum.hpp>
#include <jackbergus/data/packed.h>
namespace jackbergus {
    namespace framework {
        struct new_delta_data_structure {
            double      timestamp = 0.0;                  // Timestamp at which the current value is considered.
            uint8_t     structure_id = 0;               // Data structure containing the field described in the forthcoming steps
            type_cases  actual_type = type_cases::T_UNEXPECTED;                // Actual type determining the type that I am serializing. From this, I should be able to infer the actual datum size, as these are just native elements
            uint8_t     unnested_field_id = 0;          // Actual offset Id within the nested data structure that I am considering
            uint8_t     actual_size = 0;                // Redundant field with  actual_type, as knowing the native type should entail the actual_size
            uint8_t     is_starting_of_structure = 0;   // Under the circumstance that I am writing multiple possible fields being changed of the data structure, whether I am starting to write this. This is mainly to ensure consistency of the deserialization (if something broke, that is there is no ending part, then I am ignoring something coming afterwards).
            uint8_t     is_end_of_structure = 0;        // Under the assumption that something is starting to be written, this remarks whether I am terminating to write something concerning the data structure.
            uint8_t     is_continuing_of_structure = 0; // Whether I started to do some writing, whether I am continuing to do so
            uint8_t     CRC = 0;                        // Data validity check. If the data is no more valid, I am ignoring writing the datum, and I am reporting a nan (so to remark the difference with n/a, value not provided, and nan, error in the data)
            uint64_t    actual_data = 0;                // Buffer of arbitrary data where the values stored should be there

            static_assert(sizeof(type_cases) <= sizeof(uint8_t) && std::is_same_v<magic_enum::underlying_type_t<type_cases>, uint8_t>, "type_cases used for representing the type to be serialize should be within the uint8_t range");
            static_assert(std::numeric_limits<uint8_t>::max() >= sizeof(uint64_t), "This works under the assumption that I am representing native types being at most 64 bit long. Thus, I can fit this description within a 64 bit element");

	   static new_delta_data_structure newTimestampRecord(double timestamp) {
	   new_delta_data_structure result;
	   result.timestamp = timestamp;
	   return result;
	   }

            void setCRC() {
                CRC = *((uint64_t*)((double*)&timestamp)) ^ actual_data;
            }
        };
        static_assert(sizeof(new_delta_data_structure) == sizeof(uint64_t)*3);

        PACK(struct BlockHeader {
            FinestScaleTimeRepresentation start;
            FinestScaleTimeRepresentation end;
            uint8_t                      start_validity ;
            uint8_t                      end_validity ;
            char                         logger_record[126];
            uint64_t                     payload_size;
        });

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
                //static_assert(sizeof(BlockHeader) < block_size, "BlockHeader size mismatch");
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

            bool write(void* data_ptr, uint64_t data_size) {
                if (!stream_is_open) return false;
                else {
                    if (data_size > BUFFER_SIZE) {
                        return false; // ERROR: cannot have the data and the block to be greater than the actual block size. Something fishy is going here...
                    } else {
                        if (data_size + written_offset > BUFFER_SIZE) {
                            flush();
                        }
                        // memcpy(&buffer[written_offset], (void*)&block, sizeof(block));
                        memcpy(&buffer[written_offset], data_ptr, data_size);
                        written_offset += (data_size);
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
