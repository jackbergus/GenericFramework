//
// Created by Giacomo Bergami on 28/04/2026.
//

#include <cstdint>
#include <limits>

#include <magic_enum/magic_enum.hpp>
#include <narcissus/reflection/type_cases.h>

struct new_delta_data_structure {
    double      timestamp;                  // Timestamp at which the current value is considered.
    uint8_t     structure_id;               // Data structure containing the field described in the forthcoming steps
    type_cases  actual_type;                // Actual type determining the type that I am serializing. From this, I should be able to infer the actual datum size, as these are just native elements
    uint8_t     unnested_field_id;          // Actual offset Id within the nested data structure that I am considering
    uint8_t     actual_size;                // Redundant field with  actual_type, as knowing the native type should entail the actual_size
    uint8_t     is_starting_of_structure;   // Under the circumstance that I am writing multiple possible fields being changed of the data structure, whether I am starting to write this. This is mainly to ensure consistency of the deserialization (if something broke, that is there is no ending part, then I am ignoring something coming afterwards).
    uint8_t     is_end_of_structure;        // Under the assumption that something is starting to be written, this remarks whether I am terminating to write something concerning the data structure.
    uint8_t     is_continuing_of_structure; // Whether I started to do some writing, whether I am continuing to do so
    uint8_t     CRC;                        // Data validity check. If the data is no more valid, I am ignoring writing the datum, and I am reporting a nan (so to remark the difference with n/a, value not provided, and nan, error in the data)
    uint64_t    actual_data;                // Buffer of arbitrary data where the values stored should be there

    static_assert(sizeof(type_cases) <= sizeof(uint8_t) && std::is_same_v<magic_enum::underlying_type_t<type_cases>, uint8_t>, "type_cases used for representing the type to be serialize should be within the uint8_t range");
    static_assert(std::numeric_limits<uint8_t>::max() >= sizeof(uint64_t), "This works under the assumption that I am representing native types being at most 64 bit long. Thus, I can fit this description within a 64 bit element");
};

static_assert(sizeof(new_delta_data_structure) == sizeof(uint64_t)*3);

int main(void) {
    // Creating a new approach, which is more saving on the file end side
    // If I have multiple fields that I have to serialize, while I am not having a perfect information about everything,
    // I need the following information
    //  - At time zero, I need to know the basic data information, which is the initial information from which I am computing the delta
    //  - I am considering only native data types
    //  - I am considering pointwise events, thus data elements occurring at a given element and place in time
    // Thus now, I am serializing within the same file, contiguously, all the variables that are changing, rather than providing some validity time delta period.
    // Also, please observe that I can bring this reasoning to its extremes, by simply assuming that all the data structures for all the files are just written into one single file, so to avoid the possibility of the os mismanaging multiple small files containing almost no data at all.
    // Thus, I am simply pipelining the changes into one single file.
}