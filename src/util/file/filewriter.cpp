// util/file/filewriter.cpp -- The FileWriter class handles writing binary data to various data files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <cstdint>
#include <filesystem>

#include "util/file/binpath.hpp"
#include "util/file/fileutils.hpp"
#include "util/file/filewriter.hpp"

namespace westgate {

// Closes the binary file.
void FileWriter::close_file() { file_out_.close(); }

// Opens a file for writing.
void FileWriter::open_file(std::string filename)
{
    filename = BinPath::game_path(filename);
    std::filesystem::remove(filename);
    file_out_.open(filename.c_str(), std::ios::binary | std::ios::out);
}

// Writes binary data (in the form of an std::vector<char>) to the binary file.
void FileWriter::write_char_vec(std::vector<char> vec)
{
    write_data<uint32_t>(vec.size());
    file_out_.write(vec.data(), vec.size());
}

// Writes a string to the file.
void FileWriter::write_string(std::string str)
{
    uint32_t len = str.size();
    write_data<uint32_t>(len);
    file_out_.write(str.c_str(), str.size());
}

}   // namespace westgate
