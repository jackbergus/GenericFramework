#include <iostream>

#include <random>
#include <set>

#include <jackbergus/framework/ndp/AnyStructMonitoring.h>
#include <jackbergus/framework/ndp/FileBlockReader.h>

#include <fkYAML/node.hpp>

struct ExampleStructure {
    uint64_t a, b, c, d, e;
};
REFL_AUTO(type(ExampleStructure), field(a), field(b), field(c), field(d), field(e))


struct RecordFileDeserializer {
    RecordFileDeserializer(const std::string& name) {
        std::ifstream f{name};
        auto node = fkyaml::node::deserialize(f);
        const auto& struct_name = node["name"];
        auto& m = node["fields"].as_map();
        std::vector<jackbergus::framework::FileBlockReader<>> file_wrappers;
        std::vector<jackbergus::framework::FileBlockWrapper<>> file_block_buffers;
        std::vector<std::string> field_names;
        file_wrappers.resize(m.size());
        file_block_buffers.resize(m.size());
        field_names.resize(m.size());
        uint64_t i = 0;
        for (auto& [k, v] : node["fields"].as_map()) {
            const auto& field_name = k.as_str();

            {
                // Doing a first read, and getting all the shared temporal indices....
                jackbergus::framework::FileBlockReader<> tmp_file(v["binary"].as_str());
                uint64_t total = 0;
                while (tmp_file.read(tmp_buffer)) {
                    total += tmp_buffer.size();
                    for (uint64_t i = 0, N = tmp_buffer.size(); i < N; ++i) {
                        auto cp = tmp_buffer.get(i);
                        time_arrow.insert(cp.first->start);
                        time_arrow.insert(cp.first->end);
                    }
                }
            }

            // Now, preparing to actually read all the files
            auto& ref = file_wrappers[i];
            auto& buffer = file_block_buffers[i];
            ref.open(v["binary"].as_str()); // Opening the binary file for reading
            auto type = magic_enum::enum_cast<type_cases>(v["field_type"].as_str()).value();
            auto n_size = v["field_type_native_size"].as_int();
            auto k_name = v["field_name"].as_str();
            field_names[i] = struct_name.as_str()+"."+k_name;
            if (!ref.read(buffer)) {
                is_good = false;
            }
            i++;
        }
    }

private:
    bool is_good = true;
    std::set<jackbergus::framework::FinestScaleTimeRepresentation> time_arrow;
    jackbergus::framework::FileBlockWrapper<> tmp_buffer;
    std::vector<jackbergus::framework::FileBlockReader<>> file_wrappers;
    std::vector<jackbergus::framework::FileBlockWrapper<>> file_block_buffers;
    std::vector<std::string> field_names;
};


// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {



    std::random_device rd; // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<uint64_t> dista(10, 55); // Inclusive range

    {
        jackbergus::framework::AnyStructMonitoring<ExampleStructure> obj(0);
        obj.setFile("NitakungodeaMilele");
        ExampleStructure stru;
        for (uint64_t i = 0; i<100; i++) {
            stru.a = dista(gen);
            std::cout << stru.a << std::endl;
            stru.b = dista(gen);
            stru.c = dista(gen);
            stru.d = dista(gen);
            stru.e = dista(gen);
            obj.updateValue(i, stru);
        }
        obj.clearFile();
    }


    jackbergus::framework::FileBlockReader<> flip("NitakungodeaMilele_a[T_U_INTEGRAL[8]].bin");
    jackbergus::framework::FileBlockWrapper<> block;
    uint64_t total = 0;
    while (flip.read(block)) {
        total += block.size();
        for (uint64_t i = 0, N = block.size(); i < N; ++i) {
            auto cp = block.get(i);
            std::cout << *((uint64_t*)cp.second) << "@ [" << cp.first->start << ", " << cp.first->end << "]" << std::endl;
        }
    }


    bool is_good = true;
    std::set<jackbergus::framework::FinestScaleTimeRepresentation> time_arrow;
    jackbergus::framework::FileBlockWrapper<> tmp_buffer;


    std::ifstream f{"NitakungodeaMilele.yaml"};
    auto node = fkyaml::node::deserialize(f);
    const auto& struct_name = node["name"];
    auto& m = node["fields"].as_map();
    std::vector<jackbergus::framework::FileBlockReader<>> file_wrappers;
    std::vector<jackbergus::framework::FileBlockWrapper<>> file_block_buffers;
    std::vector<std::string> field_names;
    file_wrappers.resize(m.size());
    file_block_buffers.resize(m.size());
    field_names.resize(m.size());
    uint64_t i = 0;
    for (auto& [k, v] : node["fields"].as_map()) {
        const auto& field_name = k.as_str();

        {
            // Doing a first read, and getting all the shared temporal indices....
            jackbergus::framework::FileBlockReader<> tmp_file(v["binary"].as_str());
            uint64_t total = 0;
            while (tmp_file.read(tmp_buffer)) {
                total += tmp_buffer.size();
                for (uint64_t i = 0, N = tmp_buffer.size(); i < N; ++i) {
                    auto cp = tmp_buffer.get(i);
                    time_arrow.insert(cp.first->start);
                    time_arrow.insert(cp.first->end);
                }
            }
        }

        // Now, preparing to actually read all the files
        auto& ref = file_wrappers[i];
        auto& buffer = file_block_buffers[i];
        ref.open(v["binary"].as_str()); // Opening the binary file for reading
        auto type = magic_enum::enum_cast<type_cases>(v["field_type"].as_str()).value();
        auto n_size = v["field_type_native_size"].as_int();
        auto k_name = v["field_name"].as_str();
        field_names[i] = struct_name.as_str()+"."+k_name;
        if (!ref.read(buffer)) {
            is_good = false;
        }
        i++;
    }


    for (const auto& time_point : time_arrow) {

    }

    return 0;
}