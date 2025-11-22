// util/file/filereader.cpp -- The FileReader class provides generic methods for reading from a binary data file.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>
#include <fstream>

#include "core/core.hpp"
#include "util/file/filereader.hpp"
#include "util/file/fileutils.hpp"

using std::ios;
using std::runtime_error;
using std::string;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {

// Loads a data file into memory.
FileReader::FileReader(string filename, bool allow_missing_file) : read_index_(0)
{   
    if (!fs::exists(filename))
    {
        if (allow_missing_file) return;
        else throw runtime_error("Cannot load file: " + filename);
    }
    std::ifstream file(filename, ios::binary | ios::ate);
    if (!file.is_open()) throw runtime_error("Cannot load file: " + filename);
    std::streampos file_size = file.tellg();
    file.seekg(0, ios::beg);
    data_.resize(static_cast<size_t>(file_size));
    file.read(data_.data(), file_size);
    file.close();
}

// Reads two bytes and compares them to the standard footer.
bool FileReader::check_footer()
{
    uint8_t check[2];
    check[0] = read_data<uint8_t>();
    check[1] = read_data<uint8_t>();
    return (check[0] == 0x13 && check[1] == 0x51);
}

bool FileReader::check_header()
{
    uint8_t check[3];
    check[0] = read_data<uint8_t>();
    check[1] = read_data<uint8_t>();
    check[2] = read_data<uint8_t>();
    return (check[0] == 0xC0 && check[1] == 0xFF && check[2] == 0xEE);
}

// Reads a blob of binary data, in the form of a std::vector<char>
vector<char> FileReader::read_char_vec()
{
    const uint32_t size = read_data<uint32_t>();
    vector<char> buffer(data_.begin() + read_index_, data_.begin() + read_index_ + size);
    read_index_ += size;
    return buffer;
}

// Reads a string from the loaded file.
string FileReader::read_string()
{
    uint32_t len = read_data<uint32_t>();
    string result(data_.begin() + read_index_, data_.begin() + read_index_ + len);
    read_index_ += len;
    return result;
}

}   // namespace westgate
