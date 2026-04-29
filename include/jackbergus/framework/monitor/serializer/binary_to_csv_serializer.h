// binary_to_csv_serializer.h
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
//
// Created by Giacomo Bergami, PhD on 29/04/2026.
//

#ifndef GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H
#define GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H

#include <vector>
#include <string>

/**
 *
 * @param binary_file Single binary file containing the information pertaining to all the structures. This shall have an associated .yaml file, which provides additional information (e.g.) for enumerated data structures
 * @param final_csv_file Target file where this one shall be written
 * @param admissible_headers_for_serialization  Which relevant headers are of interest to be written in the final file
 */
void convert_binary_to_csv(const std::string &binary_file,
                           const std::string &final_csv_file,
                           std::vector<std::string> &admissible_headers_for_serialization);

#endif //GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H