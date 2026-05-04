// FileBlockReader.h
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

#ifndef GENERALFRAMEWORK_FILEBLOCKREADER_H
#define GENERALFRAMEWORK_FILEBLOCKREADER_H
#include <cstdint>
#include <string>

#include <jackbergus/framework/ndp/FileBlockWrapper.h>

namespace jackbergus {
    namespace framework {
        /**
         * Structured wrapper for reading a file previously written via FileSerializer.
         * @tparam block_size
         *
         */
        template <uint64_t block_size = 1024>
        struct FileBlockReader {
            FileBlockReader() : filp{nullptr} {}
            FileBlockReader(const std::string& filename) : filp{nullptr} {
                open(filename);
            }

            void open(const std::string& filename) {
                if (filp) {
                    fclose(filp);
                }
                filp = nullptr;
                filp = fopen(filename.c_str(), "rb");
            }

            void close() {
                if (filp) {
                    fclose(filp);
                }
                filp = nullptr;
            }

            bool read(FileBlockWrapper<block_size>& out, bool blockHeadIndexer = true) {
                if (!filp)
                    return false;
                out.next(filp, blockHeadIndexer);
                return out.isValidInput();
            }

            ~FileBlockReader() {
                close();
            }

        private:
            FILE* filp;
        };

    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_FILEBLOCKREADER_H