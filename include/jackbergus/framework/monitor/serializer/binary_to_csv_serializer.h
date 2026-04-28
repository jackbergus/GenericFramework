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

/**
 * This command delets the yaml file containing the information pointed by the previously stored data, as well as the binaries that are associated with it
 *
 * @param file_name Yaml file to be removed, alongside with its serialized binary files
 * @return True if the yaml file and the binaries pointed by it exists and the deletion of both type of files was successful as a whole, and false otherwise
 */
bool clearYamlWithBinaries(const std::string& file_name);

/**
 * This command deletes all the yaml files for binary storage and indexing that are stored within a current folder.
 *
 * @param folder Folder where to remove all the yaml files stored in it, without crawling within the subfolders.
 * @return True if all the yaml files being stored are corrected deleted according to clearYamlWithBinaries, and false otherwise
 */
bool clearFolderWithYamls(const std::string& folder);

#endif //GENERALFRAMEWORK_BINARY_TO_CSV_SERIALIZER_H