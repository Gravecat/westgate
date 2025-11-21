// util/file/filewriter.hpp -- The FileWriter class handles writing binary data to various data files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace westgate {

class FileWriter {
public:
            FileWriter() = delete;                      // No default constructor.
            FileWriter(const std::string& filename);    // Constructor, opens a binary file.
            ~FileWriter();                              // Destructor, closes any open binary files.
    void    write_char_vec(std::vector<char> vec);      // Writes binary data (in the form of an std::vector<char>) to the binary file.
    void    write_string(std::string str);              // Writes a string to the file.

    // Writes a basic data type (integer, float, etc.) to the file.
    template<typename T> void   write_data(T data)
    { file_out_.write(reinterpret_cast<const char*>(&data), sizeof(T)); }

private:
    std::ofstream   file_out_;  // File handle for writing into the binary data file.
};

}   // namespace westgate
