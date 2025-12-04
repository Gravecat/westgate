// util/filex.hpp -- Various utility functions that deal with reading and writing files.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#pragma once
#include "core/pch.hpp" // precompiled header

#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>

namespace westgate {

class FileReader {
public:
                        FileReader() = delete;  // No default constructor.
                        FileReader(std::string filename, bool allow_missing_file = false);  // Loads a data file into memory.
    [[nodiscard]] bool  check_footer();     // Reads two bytes and compares them to the standard footer.
    [[nodiscard]] bool  check_header();     // Reads three bytes and compares them to the standard header.
    std::vector<char>   read_char_vec();    // Reads a blob of binary data, in the form of a std::vector<char>
    std::string         read_string();      // Reads a string from the loaded file.

                        // Throws a std::runtime_error exception with a standardized error string.
    static void         standard_error(const std::string &err, int64_t data = 0, int64_t expected_data = 0, std::vector<std::string> error_sources = {});

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

private:
    std::vector<char>   data_;          // The data file loaded into memory.
    uint32_t            read_index_;    // The current read position in the file.
};

class FileWriter {
public:
            FileWriter() = delete;                      // No default constructor.
            FileWriter(const std::string& filename);    // Constructor, opens a binary file.
            ~FileWriter();                              // Destructor, closes any open binary files.
    void    write_char_vec(std::vector<char> vec);      // Writes binary data (in the form of an std::vector<char>) to the binary file.
    void    write_footer();                             // Writes a standard EOF footer, so the game can confirm the file ends where it should.
    void    write_header();                             // Writes a standard header, so the game can identify its own files.
    void    write_string(std::string str);              // Writes a string to the file.

    // Writes a basic data type (integer, float, etc.) to the file.
    template<typename T> void   write_data(T data)
    { file_out_.write(reinterpret_cast<const char*>(&data), sizeof(T)); }

private:
    std::ofstream   file_out_;  // File handle for writing into the binary data file.
};

class FileX {
public:
    // Fags for file_to_vec()
    static constexpr uint8_t    FTV_FLAG_IGNORE_BLANK_LINES =   1;  // Ignores any blank lines in the file.
    static constexpr uint8_t    FTV_FLAG_IGNORE_COMMENTS =      2;  // Ignores any comments (lines starting with #) in the file.
    static constexpr uint8_t    FTV_FLAG_NO_STRIP_NEWLINES =    4;  // Tells file_to_vec() to NOT strip trailing \r or \n newlines from the file.

    static std::string  file_to_string(const std::string& filename);    // Loads a text file into an std::string.
    // Loads a text file into a vector, one string for each line of the file.
    static std::vector<std::string> file_to_vec(const std::string& filename, uint8_t flags = 0);
};

}   // westgate namespace
