//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_ANYSTRUCTMONITORING_H
#define GENERALFRAMEWORK_ANYSTRUCTMONITORING_H

#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <vector>

//#include <field_reflection.hpp> Removed. as it was not compatible with C++17 TODO: conditional compiling depending on the version of C++
#include "refl.hpp"

#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h>

#include "jackbergus/data/recursive_encoder.h"

#include <fkYAML/node.hpp>

namespace jackbergus {
    namespace framework {

        template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_for {
            std::vector<AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepresentation start_time) {
                auto result = static_for<T, x+1,to>().expandWithBasicMonitor(start_time);
                auto view = std::string(refl::trait::get_t<x, refl::member_list<T>>::name.c_str());
                using K = typename refl::trait::get_t<x, refl::member_list<T>>::value_type;
                result.emplace_back(flatten_type_to_enum<K>(start_time, x, view));
                return result;
            }

            bool setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepresentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f) {
                auto val_rec = static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f);
                auto val = get_field_t<T, x>::get(value);
                // auto val = field_reflection::get_field<x>(value);
                return f[x].updateValue(start_time, val) ? val_rec : false;
            }
        };

        template<typename T, int to, uint64_t block_size>
        struct static_for<T, to,to, block_size> {
            std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepresentation start_time) {
                return {};
            }

            bool setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepresentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f) {
                return true;
            }
        };

        template <typename  T, uint64_t block_size=1024> std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> getNativeType(jackbergus::framework::FinestScaleTimeRepresentation start_time) {
            auto v = static_for<T, 0, refl::member_list<T>::size>{}.expandWithBasicMonitor(start_time);
            std::reverse(v.begin(), v.end());
            return v;
        }

        /**
         * Serializes and persists a record by serializing each field separately
         * @tparam T              Struct of native types to serialize
         * @tparam block_size
         */
        template <typename  T, uint64_t block_size = 1024>
                class AnyStructMonitoring : public jackbergus::framework::ContinuousMonitoring<T> {

            std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> fields;
            std::string FileName_;
        public:
            constexpr static uint64_t field_count = refl::member_list<T>::size; //field_reflection::field_count<T>;
            using validity_vector = std::array<bool, field_count>;

            [[nodiscard]] const FinestScaleTimeRepresentation getCurrentTime() const  override {
                FinestScaleTimeRepresentation t = 0;
                for (const auto& ref : fields) {
                    if (ref.validityInterval().second > t) {
                        t = ref.validityInterval().second;
                    }
                }
                return t;
            };
            [[nodiscard]] bool isCurrentlyValid() const override {
                for (const auto& ref : fields) {
                    if (ref.isCurrentlyValid()) {
                        return true;
                    }
                }
                return false;
            }
            [[nodiscard]] virtual void* getRawPtr() const {
                throw std::invalid_argument("getRawPtr()");
            }

            bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t) override {
                auto result = true;
                for (auto& ref : fields) {
                    if (!ref.setInvalidValue(curr_t))
                        result = false;
                }
                return result;
            }

            bool updateValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t,
                             const T& value)  override {
                return static_for<T, 0, refl::member_list<T>::size>{}.setRecursivelyWithTemplates(curr_t, value, fields);
            }

            AnyStructMonitoring(jackbergus::framework::FinestScaleTimeRepresentation start_time) {
                fields = getNativeType<T>(start_time);
                setInvalidValue(start_time);
            }

            AnyStructMonitoring(jackbergus::framework::FinestScaleTimeRepresentation start_time, const T& value) {
                fields = getNativeType<T>(start_time);
                updateValue(start_time, value);
            }

            virtual ~AnyStructMonitoring() {
                clearFile();
            }

            void clearFile() override {
                fkyaml::node node = {{"name", FileName_},  {"fields", fkyaml::node::mapping()}};
                auto& field_struct = node["fields"].as_map();
                for (auto& ref : fields) {
                    std::string val = ref.field_name();
                    auto struct_field = fkyaml::node::mapping();
                    struct_field["field_name"] = val;
                    struct_field["field_type"] = std::string( magic_enum::enum_name(ref.field_type()));
                    struct_field["field_type_native_size"] = ref.native_size();
                    struct_field["binary"] = FileName_+"_"+ref.field_name()+".bin";
                    field_struct[val] = struct_field;
                    ref.clearFile();
                }
                if (!FileName_.empty())
                {
                    std::ofstream f{FileName_+".yaml"};
                    f << node << std::endl;
                }
            }

            void setFile(const std::string& FileName) override {
                clearFile();
                FileName_ = FileName;
                for (auto& field : fields) {
                    field.setFile(FileName+"_"+field.field_name()+".bin");
                }
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_ANYSTRUCTMONITORING_H