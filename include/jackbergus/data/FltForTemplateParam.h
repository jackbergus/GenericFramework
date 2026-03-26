// FltForTemplateParam.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - Giacomo Bergami
//
// GeneralFramework is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  GeneralFramework is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GeneralFramework. If not, see <http://www.gnu.org/licenses/>.

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