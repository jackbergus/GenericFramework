#include <iostream>

#include <random>

#include "jackbergus/framework/monitor/ContinuousVariableMonitoring.h"
#include "field_reflection.hpp"


#include <narcissus/reflection/type_cases.h>
#include <typeindex>

#include "narcissus/lightweight_any.h"
#include "narcissus/template_typing.h"

#include <fstream>





#include <cassert>
#include <magic_enum/magic_enum.hpp>

#include "jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h"




template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_for {
    std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
        auto result = static_for<T, x+1,to>().expandWithBasicMonitor(start_time);
        auto view = field_reflection::field_name<T, x>;
        result.emplace_back(flatten_type_to_enum<field_reflection::field_type<T, x>>(start_time, x, std::string(view.data(), view.size())));
        return result;
    }

    bool setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepreentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f) {
        auto val_rec = static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f);
        auto val = field_reflection::get_field<T, x>(value);
        return f[x].updateValue(start_time, val) ? val_rec : false;
    }
};

template<typename T, int to, uint64_t block_size>
struct static_for<T, to,to, block_size> {
    std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
        return {};
    }

    bool setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepreentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f) {
        return true;
    }
};

struct ExampleStructure {
    uint64_t a, b, c, d, e;
};

#include <cstdio>
#include <algorithm>

template <typename  T, uint64_t block_size=1024> std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> getNativeType(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
    auto v = static_for<T, 0, field_reflection::field_count<T>>{}.expandWithBasicMonitor(start_time);
    std::reverse(v.begin(), v.end());
    return v;
}

template <typename  T, uint64_t block_size = 1024>
        class AnyStructMonitoring : public jackbergus::framework::ContinuousMonitoring {

    std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> fields;
public:
    constexpr static uint64_t field_count = field_reflection::field_count<T>;
    using validity_vector = std::array<bool, field_count>;

    bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t) override {
        auto result = true;
        for (auto& ref : fields) {
            if (!ref.setInvalidValue(curr_t))
                result = false;
        }
        return result;
    }

    bool updateValue(jackbergus::framework::FinestScaleTimeRepreentation curr_t,
                     const T& value) {
        return static_for<T, 0, field_reflection::field_count<T>>{}.setRecursivelyWithTemplates(curr_t, value, fields);
    }

    AnyStructMonitoring(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
        fields = getNativeType<T>(start_time);
        setInvalidValue(start_time);
    }

    AnyStructMonitoring(jackbergus::framework::FinestScaleTimeRepreentation start_time, const T& value) {
        fields = getNativeType<T>(start_time);
        updateValue(start_time, value);
    }

    void clearFile() override {
        for (auto& ref : fields) {
            ref.clearFile();
        }
    }

    void setFile(const std::string& FileName) override {
        clearFile();
        for (auto& field : fields) {
            field.setFile(FileName+"_"+field.field_name()+".bin");
        }
    }
};




// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    return 0;

    // FILE* filp = fopen(binary, "rb" );
    // if (!filp) { printf("Error: could not open file %s\n", binary); return -1; }
    //
    // char * buffer[1024];
    // int bytes;
    // memset(buffer, 0, sizeof(buffer));
    // uint64_t total = 0;
    // while ( (bytes = fread(buffer, sizeof(char), 1024, filp)) > 0 ) {
    //     if (bytes == 1024) {
    //         uint64_t N_elements_in_block = *(uint64_t *) buffer;
    //         total += N_elements_in_block;
    //         jackbergus::framework::BlockHeader* first_header = (jackbergus::framework::BlockHeader*)(((uint64_t *) buffer)+1);
    //         for (uint64_t i = 0; i < N_elements_in_block; ++i) {
    //             double val = *(double*)(((char*)first_header)+sizeof(jackbergus::framework::BlockHeader));
    //             std::cout << val << "@ [" << first_header->start << ", " << first_header->end << "]" << std::endl;
    //             first_header = (jackbergus::framework::BlockHeader*)(((char*)first_header)+sizeof(jackbergus::framework::BlockHeader)+first_header->payload_size);
    //         }
    //
    //     }
    //     memset(buffer, 0, sizeof(buffer));
    // }
    //         std::cout << total << std::endl;

    // Done and close.
    // fclose(filp);

    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}