//
// Created by gyankos on 24/03/26.
//

#ifndef GENERALFRAMEWORK_UPDATABLEELEMENT_H
#define GENERALFRAMEWORK_UPDATABLEELEMENT_H

#include "jackbergus/data/template_typing.h"
#include "jackbergus/data/recursive_encoder.h"

template <class LogicalRepresentation,
          class RawRepresentation>
struct UpdatableElement : public FilterMap {
    LogicalRepresentation data;

    UpdatableElement() {
        field_names = collect_field_names(data);
    }

    const std::vector<std::string>& get_fields() const {
        return field_names;
    }

    ~UpdatableElement() override {

    }

    void serialize_to_csv(std::ostream& stream, const std::vector<std::string>& header_order) {
        std::unordered_map<std::string, std::string> values;
        serialize(data, values);
        for (uint64_t i = 0; i < header_order.size(); i++) {
           auto it = values.find(header_order[i]);
           if (it == values.end()) {
               stream << "n/a";
           } else {
               stream << it->second;
           }
           if (i < header_order.size() - 1) {
               stream << ",";
           }
        }
    }

    /**
     * Checks whether the current memory id is activated
     * @param bit   Memory id to check whether it gets to be updated
     * @return      Whether the memory is activated or not
     */
    bool isCurrentElementInMap(uint64_t bit) override {
        return (((uint64_t)1 << bit) & map_) == ((uint64_t)1 << bit);
    }

    /**
     * Activates the updates for a specific memory index
     * @param bit   Memory Id to be activated
     */
    void setBit(uint64_t bit) override {
        if (bit >= counter) {
            map_ = (map_ | ((uint64_t)1 << bit));
        }
    }

    /**
     * Deactivates the updates for a specific memory index
     * @param bit memory not to be updated with forthcoming data updates
     */
    void offbit(uint64_t bit) override {
        if (bit >= counter) {
            map_ = (map_ & ~((uint64_t)1 << bit));
        }
    }
    void clearBits() {
        map_ = 0;
    }

    /**
     * Associates the field object with some external memory
     * @param ptr Raw memory to be updated with the values coming from the
     * @return    Id associated with the current memory position, so that it can be used as a bit in a bitmap for setting the memory in this...
     */
    uint64_t associateWithRawMemory(RawRepresentation* ptr) {
        addr_to_id[ptr] = counter;
        all_bits = (all_bits | ((uint64_t)1 << counter));
        encode_extended(ptr, data, counter, this);
        auto val = counter;
        counter++;
        return val;
    }

    /**
     * Forces the broadcast to all
     */
    void forceBroadcast() {
        auto all_data = map_;
        map_ = 0xFFFFFFFF; // Forcing all the bits to be activated, thus forcing the
        update(data);
        map_ = all_data;
    }

    /**
     * Updates the logical representation from a low level pointer
     * @param ptr                   Memory containing low-level representation
     * @param updateOtherElements   Whether the received update should be broadcasted to the other triggered elements
     */
    void decodeFrom(const RawRepresentation& ptr, bool updateOtherElements = true) {
        uint64_t current_map;
        if (updateOtherElements) {
            current_map = all_bits;
            auto it = addr_to_id.find(ptr);
            if (it != addr_to_id.end()) {
                current_map = (current_map & ~((uint64_t)1 << it->second));
            }
        } else {
            current_map = 0;
        }
        std::swap(current_map, map_);
        decode(data, ptr);
        map_ = current_map;
    }

private:
    std::unordered_map<RawRepresentation*, uint64_t> addr_to_id;
    std::vector<std::string> field_names;
    uint64_t counter = 0;
    uint64_t map_ = 0;
    uint64_t all_bits = 0;
};

#endif //GENERALFRAMEWORK_UPDATABLEELEMENT_H