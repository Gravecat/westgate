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

// Constructor, opens a binary file.
FileWriter::FileWriter(const std::string& filename)
{
    const std::string bp_filename = BinPath::game_path(filename);
    std::filesystem::remove(bp_filename);
    file_out_.open(bp_filename.c_str(), std::ios::binary | std::ios::out);
}

// Destructor, closes any open binary files.
FileWriter::~FileWriter() { file_out_.close(); }

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
