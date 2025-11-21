// util/file/filereader.hpp -- The FileReader class provides generic methods for reading from a binary data file.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>
#include <cstring>  // std::memcpy
#include <string>
#include <vector>

namespace westgate {

class FileReader {
public:
                        FileReader() = delete;  // No default constructor.
                        FileReader(std::string filename, bool allow_missing_file = false);  // Loads a data file into memory.
    std::vector<char>   read_char_vec();    // Reads a blob of binary data, in the form of a std::vector<char>
    std::string         read_string();      // Reads a string from the loaded file.

    // Reads data from a loaded file.
    template<typename T> T  read_data()
    {
        if (read_index_ + sizeof(T) > data_.size()) throw std::runtime_error("Attmept to read out-of-bounds data!");
        char* mid_pos = data_.data() + read_index_;
        T result;
        std::memcpy(&result, mid_pos, sizeof(T));
        read_index_ += sizeof(T);
        return result;
    }

protected:
    std::vector<char>   data_;          // The data file loaded into memory.
    uint32_t            read_index_;    // The current read position in the file.
};

}   // namespace westgate
