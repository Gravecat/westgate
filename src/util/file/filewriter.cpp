// util/file/filewriter.cpp -- The FileWriter class handles writing binary data to various data files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>

#include "util/file/binpath.hpp"
#include "util/file/fileutils.hpp"
#include "util/file/filewriter.hpp"

using std::ios;
using std::string;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {

// Constructor, opens a binary file.
FileWriter::FileWriter(const string& filename)
{
    const string bp_filename = BinPath::game_path(filename);
    fs::remove(bp_filename);
    file_out_.open(bp_filename.c_str(), ios::binary | ios::out);
}

// Destructor, closes any open binary files.
FileWriter::~FileWriter() { file_out_.close(); }

// Writes binary data (in the form of an std::vector<char>) to the binary file.
void FileWriter::write_char_vec(vector<char> vec)
{
    write_data<uint32_t>(vec.size());
    file_out_.write(vec.data(), vec.size());
}

// Writes a standard EOF footer, so the game can confirm the file ends where it should.
void FileWriter::write_footer()
{
    write_data<uint8_t>(0x13);
    write_data<uint8_t>(0x51);
}

// Writes a standard header string, so the game can identify its own files.
void FileWriter::write_header()
{
    write_data<uint8_t>(0xC0);
    write_data<uint8_t>(0xFF);
    write_data<uint8_t>(0xEE);
}

// Writes a string to the file.
void FileWriter::write_string(string str)
{
    uint32_t len = str.size();
    write_data<uint32_t>(len);
    file_out_.write(str.c_str(), str.size());
}

}   // namespace westgate
