//
// Created by Giacomo Bergami on 28/04/2026.
//



#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoringWithSharedFile.h>

#include <magic_enum/magic_enum.hpp>

#include "admissible_nested_example.h"
#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h>
#include <jackbergus/framework/monitor/AnyStructMonitoring.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>

#include <jackbergus/framework/monitor/serializer/binary_to_csv_serializer.h>

#include <jackbergus/framework/monitor/deserializer/FileLogger.h>
//#include <sys/unistd.h>


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

    // But first, I am giving some trivial example of how lightweight_any works
    // This enables any casting, while preserving the type information....!
    // Also, it enables storing fundamental types while returning the underlying data as expected.
    {
        lightweight_any out;
        {
            uint64_t valuer{123};
            lightweight_any val{valuer};
            out = val;
        }
        std::cout << *(uint64_t *) out.raw() << std::endl;
        {
            double valued{123.456};
            lightweight_any val{valued};
            out = val;
        }
        std::cout << *(double *) out.raw() << std::endl;
        {
            uint8_t valuer{90};
            lightweight_any val{valuer};
            out = val;
        }
        std::cout << (uint64_t)*(uint8_t *) out.raw() << std::endl;
    }


    std::string binary_file = "some_binary_file.bin";
    std::string yaml_file = binary_file + ".yaml";


    {
        FileLogger<> logger_test{binary_file};

        Final_F final_n;
        memset(&final_n, 0, sizeof(final_n));
        logger_test.registerObjectWithName("final_n_msg", final_n);

        BogusConcurrentDataRecord concurrent;
        memset(&concurrent, 0, sizeof(concurrent));
        logger_test.registerObjectWithName("concurrent", concurrent);

        // Memento: after registering all the elements, then serializing the yaml file acting as a documentation index of the data structures of interest
        logger_test.write_yaml_configuration();

        concurrent.val = 1; // valori dal tempo 1
        logger_test.updateStruct("concurrent", 1, concurrent);

        // From now onwards, simulating some elements being changed
        // Starting with small changes
        final_n.enumerato = 7; // Valori dal tempo 2
        final_n.third = 86; // Valori dal tempo 2
        logger_test.updateStruct("final_n_msg", 2, final_n);

        concurrent.timestamp = 2.0; // valori dal tempo 2
        concurrent.t = VAL_1;
        logger_test.updateStruct("concurrent", 2, concurrent);

        concurrent.val = 3; // valori dal tempo 3
        logger_test.updateStruct("concurrent", 3, concurrent);

        // Now, considering nested changes, forsooth!
        final_n.first.cho = 5; // Valori dal tempo 4
        final_n.first.jes = 6; // Valori dal tempo 4
        final_n.first.val = 7; // Valori dal tempo 4
        logger_test.updateStruct("final_n_msg", 4, final_n);

        final_n.second[5].cho = 11; // Valori dal tempo 5
        logger_test.updateStruct("final_n_msg", 5, final_n);

        concurrent.timestamp = 4.0; // valori dal tempo 5
        logger_test.updateStruct("concurrent", 5, concurrent);

        final_n.second[6].cho = 13; // Valori dal tempo 6
        logger_test.updateStruct("final_n_msg", 6, final_n);

        final_n.enumerato2 = 3; // Valori dal tempo 2
        final_n.second[7].cho = 19; // Valori dal tempo 7
        logger_test.updateStruct("final_n_msg", 7, final_n);

        final_n.first.voi_ = 8; // Valori dal tempo 8
        final_n.enumerato3 = 1; // Valori dal tempo 2
        logger_test.updateStruct("final_n_msg", 8, final_n);

        concurrent.val = 9; // valori dal tempo 9
        logger_test.updateStruct("concurrent", 9, concurrent);

        logger_test.flush();
        logger_test.close();
    }

    std::string final_csv_file = "final_csv_file_v4.csv";
    std::vector<std::string> admissible_headers_for_serialization{
        "concurrent.t",
        "concurrent.val", "concurrent.timestamp", "final_n_msg.enumerato", "final_n_msg.third", "final_n_msg.first.cho",
        "final_n_msg.first.jes", "final_n_msg.first.val", "final_n_msg.first.voi_", "final_n_msg.second[5].cho",
        "final_n_msg.second[6].cho", "final_n_msg.second[7].cho"
    };

    // Testing serializing everything
    // admissible_headers_for_serialization.clear();

    sort_and_convert_binary_to_csv(binary_file, final_csv_file, admissible_headers_for_serialization);
    std::remove(binary_file.c_str());
    std::remove(yaml_file.c_str());
}
