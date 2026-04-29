//
// Created by mbda on 27/04/2026.
//

#include <narcissus/lightweight_any.h>
#include "admissible_nested_example.h"
#include "jackbergus/framework/monitor/AnyStructMonitoring.h"
#include "jackbergus/framework/monitor/serializer/binary_to_csv_serializer.h"


int main() {
     {
          Final_N final_n;
          memset(&final_n, 0, sizeof(final_n));
          auto rs = jackbergus::framework::getNativeType<Final_N>(0);
          std::cout << "Final_N: " << std::endl;
          std::cout << "=========" << std::endl;
          for (const auto& ref : rs) {
               std::cout << ref.field_name() << std::endl;
          }


          jackbergus::framework::AnyStructMonitoring<Final_N> mimicking_serialization(0); // Starting at zero with an invalid value (mimicking external configurations, that are set only once they are known).
          mimicking_serialization.setFile("Final_N");

          // From now onwards, simulating some elements being changed
          // Starting with small changes
          final_n.enumerato = 123; // Valori dal tempo 2
          final_n.third = 86;      // Valori dal tempo 2
          mimicking_serialization.updateValue(2, final_n);

          // Now, considering nested changes, forsooth!
          final_n.first.cho = 5; // Valori dal tempo 4
          final_n.first.jes = 6; // Valori dal tempo 4
          final_n.first.val = 7; // Valori dal tempo 4
          mimicking_serialization.updateValue(4, final_n);

          final_n.second[5].cho = 11; // Valori dal tempo 5
          mimicking_serialization.updateValue(5, final_n);

          final_n.second[6].cho = 13; // Valori dal tempo 6
          mimicking_serialization.updateValue(6, final_n);

          final_n.second[7].cho = 19; // Valori dal tempo 7
          mimicking_serialization.updateValue(7, final_n);

          final_n.first.voi_ = 8; // Valori dal tempo 8
          mimicking_serialization.updateValue(8, final_n);
          mimicking_serialization.clearFile();
     }

     {
          BogusConcurrentDataRecord concurrent;
          std::cout << "BogusConcurrentDataRecord: " << std::endl;
          std::cout << "===========================" << std::endl;
          memset(&concurrent, 0, sizeof(concurrent));
          auto rs = jackbergus::framework::getNativeType<BogusConcurrentDataRecord>(0);
          for (const auto& ref : rs) {
               std::cout << ref.field_name() << std::endl;
          }

          jackbergus::framework::AnyStructMonitoring<BogusConcurrentDataRecord> mimicking_serialization(0, concurrent); // starting at zero with some valid zero values (mimicking the internal and non-external configurations)
          mimicking_serialization.setFile("concurrent");

          concurrent.val = 1; // valori dal tempo 1
          mimicking_serialization.updateValue(1, concurrent);

          concurrent.timestamp = 2.0; // valori dal tempo 2
          mimicking_serialization.updateValue(2, concurrent);

          concurrent.val = 3; // valori dal tempo 3
          mimicking_serialization.updateValue(3, concurrent);

          mimicking_serialization.setInvalidValue(4);

          concurrent.timestamp = 4.0; // valori dal tempo 5
          mimicking_serialization.updateValue(5, concurrent);

          mimicking_serialization.setInvalidValue(7);

          concurrent.val = 9; // valori dal tempo 9
          mimicking_serialization.updateValue(9, concurrent);
          mimicking_serialization.clearFile();
     }

     binary_to_csv_serializer("C:\\Users\\diste\\Downloads\\GenericFramework2\\cmake-build-debug", "final_flattened_csv_file.csv");
     clearFolderWithYamls("C:\\Users\\diste\\Downloads\\GenericFramework2\\cmake-build-debug");
}
