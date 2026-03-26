//
// Created by gyankos on 11/03/26.
//

#ifndef FLOATMAX_FLTFORTEMPLATEPARAM_H
#define FLOATMAX_FLTFORTEMPLATEPARAM_H

#include <limits>

#define DBLTYPE(Name, value)    struct Name { constexpr operator double() const { return (double)(value); } };
#define FLTTYPE(Name, value)    struct Name { constexpr operator float() const  { return (float)(value); } };

FLTTYPE(FZero, 0.0)
FLTTYPE(FOne, 1.0)
FLTTYPE(FEps, std::numeric_limits<float>::epsilon())
FLTTYPE(FMax, std::numeric_limits<float>::max())


#endif //FLOATMAX_FLTFORTEMPLATEPARAM_H