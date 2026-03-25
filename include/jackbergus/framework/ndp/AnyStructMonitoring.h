//
// Created by gyankos on 04/03/26.
//

#ifndef GENERALFRAMEWORK_ANYSTRUCTMONITORING_H
#define GENERALFRAMEWORK_ANYSTRUCTMONITORING_H

#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <vector>

#include <field_reflection.hpp>

#include <jackbergus/framework/monitor/AnyFundamentalVariableMonitoring.h>

namespace jackbergus {
    namespace framework {

        template<typename T, int x, int to, uint64_t block_size = 1024>
struct static_for {
            std::vector<AnyFundamentalVariableMonitoring<block_size>> expandWithBasicMonitor(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
                auto result = static_for<T, x+1,to>().expandWithBasicMonitor(start_time);
                auto view = field_reflection::field_name<T, x>;
                result.emplace_back(flatten_type_to_enum<field_reflection::field_type<T, x>>(start_time, x, std::string(view.data(), view.size())));
                return result;
            }

            bool setRecursivelyWithTemplates(jackbergus::framework::FinestScaleTimeRepreentation start_time, const T& value, std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>>& f) {
                auto val_rec = static_for<T, x+1,to>().setRecursivelyWithTemplates(start_time, value, f);
                auto val = field_reflection::get_field<x>(value);
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

        template <typename  T, uint64_t block_size=1024> std::vector<jackbergus::framework::AnyFundamentalVariableMonitoring<block_size>> getNativeType(jackbergus::framework::FinestScaleTimeRepreentation start_time) {
            auto v = static_for<T, 0, field_reflection::field_count<T>>{}.expandWithBasicMonitor(start_time);
            std::reverse(v.begin(), v.end());
            return v;
        }

        /**
         * Serializes and persists a record by serializing each field separately
         * @tparam T              Struct of native types to serialize
         * @tparam block_size
         */
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

            virtual ~AnyStructMonitoring() {}

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
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_ANYSTRUCTMONITORING_H