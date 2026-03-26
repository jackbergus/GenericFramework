//
// Created by gyankos on 26/03/26.
//

#ifndef GENERALFRAMEWORK_RECORDFILEDESERIALIZER_H
#define GENERALFRAMEWORK_RECORDFILEDESERIALIZER_H
#include <cstdint>
#include <set>
#include <unordered_map>
#include <vector>

#include <jackbergus/framework/monitor/deserializer//FieldGetting.h>
#include <jackbergus/framework/types/NativeTypes.h>
#include <jackbergus/framework/ndp/FileSerializer.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>
#include <jackbergus/framework/ndp/FileBlockWrapper.h>

namespace jackbergus {
    namespace data {
        namespace deserializer {

struct RecordFileDeserializer {
    RecordFileDeserializer();

    const std::vector<std::string> &columns() const {
        return field_names;
    }

    const uint64_t n_variables() const {
        return field_names.size();
    }

    const FieldGetting *getFieldInfo(uint64_t idx) const {
        if (idx >= fields.size()) {
            return nullptr;
        }
        return &fields[idx];
    }

    void init(const std::string &name, std::set<jackbergus::framework::FinestScaleTimeRepresentation> &time_arrow);
    std::pair<jackbergus::framework::BlockHeader *, void *> current(uint64_t column_idx) const;
    bool next(uint64_t column_idx);

private:
    // bool is_good = true;
    std::vector<std::pair<uint64_t, uint64_t> > field_offsets;
    std::vector<bool> is_good;
    framework::FileBlockWrapper<> tmp_buffer;
    std::vector<framework::FileBlockReader<> > file_wrappers;
    std::vector<jackbergus::framework::FileBlockWrapper<> > file_block_buffers;
    std::vector<std::string> field_names;
    std::vector<FieldGetting> fields;
    std::unordered_map<std::string, uint64_t> names_to_fieldsidx;
};

        } // deserializer
    } // data
} // jackbergus

#endif //GENERALFRAMEWORK_RECORDFILEDESERIALIZER_H