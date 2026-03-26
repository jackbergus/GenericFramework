//
// Created by gyankos on 05/03/26.
//

#ifndef GENERALFRAMEWORK_ABSTRACTVARIABLEMONITOR_H
#define GENERALFRAMEWORK_ABSTRACTVARIABLEMONITOR_H

#include <jackbergus/framework/types/NativeTypes.h>


    namespace jackbergus::framework {
        template<typename Type>
        class AbstractVariableMonitor {
        public:
            virtual ~AbstractVariableMonitor() = default;
            [[nodiscard]] virtual const FinestScaleTimeRepresentation getCurrentTime() const  = 0;
            [[nodiscard]] virtual bool isCurrentlyValid() const = 0;
            [[nodiscard]] virtual void* getRawPtr() const = 0;
            virtual bool setInvalidValue(jackbergus::framework::FinestScaleTimeRepresentation curr_t) = 0;
            virtual bool updateValue(FinestScaleTimeRepresentation curr_t,
                             const Type& value) = 0;
        };
    } // framework
// jackbergus

#endif //GENERALFRAMEWORK_ABSTRACTVARIABLEMONITOR_H