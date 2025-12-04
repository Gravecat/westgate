// util/filex.cpp -- Various utility functions that deal with reading and writing files.

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

#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include "util/binpath.hpp"
#include "util/filex.hpp"
#include "util/random.hpp"
#include "util/strx.hpp"

using std::runtime_error;
using std::string;
using std::to_string;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {

/* FILEREADER */

// Loads a data file into memory.
FileReader::FileReader(string filename, bool allow_missing_file) : read_index_(0)
{   
    if (!fs::exists(filename))
    {
        if (allow_missing_file) return;
        else throw runtime_error("Cannot load file: " + filename);
    }
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) throw runtime_error("Cannot load file: " + filename);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);
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

// Throws a std::runtime_error exception with a standardized error string.
void FileReader::standard_error(const string &err, int64_t data, int64_t expected_data, vector<string> error_sources)
{
    string error_str = err;
    if (data != expected_data) error_str += " (" + to_string(data) + ", expected " + to_string(expected_data) + ")";
    if (error_sources.size()) error_str += " [" + StrX::comma_list(error_sources) + "]";
    throw runtime_error(error_str);
}

/* FILEWRITER */

// Constructor, opens a binary file.
FileWriter::FileWriter(const string& filename)
{
    const string bp_filename = BinPath::game_path(filename);
    fs::remove(bp_filename);
    file_out_.open(bp_filename.c_str(), std::ios::binary | std::ios::out);
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

/* FILEX */

// Loads a text file into an std::string.
string FileX::file_to_string(const string& filename)
{
    if (!fs::exists(filename)) throw runtime_error("Invalid file: " + filename);
    std::ifstream file(filename);
    if (!file.is_open()) throw runtime_error("Cannot open file: " + filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

// Loads a text file into a vector, one string for each line of the file.
vector<string> FileX::file_to_vec(const string& filename)
{
    if (!fs::exists(filename)) throw runtime_error("Invalid file: " + filename);
    vector<string> lines;
    std::ifstream file(filename);
    if (!file.is_open()) throw runtime_error("Cannot open file: " + filename);

    string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
        lines.push_back(line);
    }
    file.close();
    return lines;
}

}   // westgate namespace
