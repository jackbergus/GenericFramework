//
// Created by gyankos on 04/03/26.
//

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
            FileBlockReader(const std::string& filename) {
                filp = fopen(filename.c_str(), "rb");
            }

            bool read(FileBlockWrapper<block_size>& out) {
                if (!filp)
                    return false;
                out.next(filp);
                return out.isValidInput();
            }

            ~FileBlockReader() {
                if (filp) {
                    fclose(filp);
                }
                filp = nullptr;
            }

        private:
            FILE* filp;
        };

    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_FILEBLOCKREADER_H