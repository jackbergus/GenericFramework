//
// Created by gyankos on 26/03/26.
//

#ifndef GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H
#define GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H

#include <string>

/**
 * Compacts multiple binary representations in a folder into one single CSV file merging all the discrete ones
 * @param folder        Folder containing the binary files and the yaml files providing the explanation of what the binaries are about
 * @param dst_csv_file  CSV file where the results will be written
 * @return              False if either folder is a non-existing folder, or dst_csv_file is a folder, or there was any error on writing
 *                      True otherwise
 */
bool binary_to_csv_serializer(const std::string& folder,
                              const std::string& dst_csv_file);

#endif //GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H