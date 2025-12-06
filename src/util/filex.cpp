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

#if defined(WESTGATE_TARGET_APPLE)
#include <mach-o/dyld.h>    // _NSGetExecutablePath()
#elif defined(WESTGATE_TARGET_LINUX)
#include <unistd.h>         // readlink
#elif defined(WESTGATE_TARGET_WINDOWS)
#include <windows.h>        // GetModuleFileNameW
#endif

#include "util/filex.hpp"
#include "util/random.hpp"
#include "util/strx.hpp"

using std::runtime_error;
using std::string;
using std::string_view;
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
    if (read_data<uint8_t>() != 0x13) return false;
    if (read_data<uint8_t>() != 0x51) return false;
    return true;
}

bool FileReader::check_header()
{
    if (read_data<uint8_t>() != 0xC0) return false;
    if (read_data<uint8_t>() != 0xFF) return false;
    if (read_data<uint8_t>() != 0xEE) return false;

    // Check the sizes of data types; if the game was saved on a platform with weird data type sizes, like int being 16-bit, we need to know before trying to
    // load the binary data on this platform.
    if (read_data<uint8_t>() != sizeof(size_wg)) return false;
    if (read_data<uint8_t>() != sizeof(char)) return false;
    if (read_data<uint8_t>() != sizeof(short)) return false;
    if (read_data<uint8_t>() != sizeof(int)) return false;
    if (read_data<uint8_t>() != sizeof(long)) return false;
    if (read_data<uint8_t>() != sizeof(long long)) return false;
    if (read_data<uint8_t>() != sizeof(float)) return false;
    if (read_data<uint8_t>() != sizeof(double)) return false;
    if (read_data<uint8_t>() != sizeof(long double)) return false;
    if (read_data<uint8_t>() != sizeof(bool)) return false;
    return true;
}

// Reads a blob of binary data, in the form of a std::vector<char>
vector<char> FileReader::read_char_vec()
{
    const size_wg size = read_data<size_wg>();
    vector<char> buffer(data_.begin() + read_index_, data_.begin() + read_index_ + size);
    read_index_ += size;
    return buffer;
}

// Reads a string from the loaded file.
string FileReader::read_string()
{
    size_wg len = read_data<size_wg>();
    string result(data_.begin() + read_index_, data_.begin() + read_index_ + len);
    read_index_ += len;
    return result;
}

// Throws a std::runtime_error exception with a standardized error string.
void FileReader::standard_error(const string &err, int64_t data, int64_t expected_data, vector<string> error_sources)
{
    string error_str = err;
    if (data != expected_data) error_str += " (" + to_string(data) + ", expected " + to_string(expected_data) + ")";
    if (error_sources.size()) error_str += " [" + strx::comma_list(error_sources) + "]";
    throw runtime_error(error_str);
}

/* FILEWRITER */

// Constructor, opens a binary file.
FileWriter::FileWriter(const string& filename)
{
    const string bp_filename = filex::game_path(filename);
    fs::remove(bp_filename);
    file_out_.open(bp_filename.c_str(), std::ios::binary | std::ios::out);
}

// Destructor, closes any open binary files.
FileWriter::~FileWriter() { file_out_.close(); }

// Writes binary data (in the form of an std::vector<char>) to the binary file.
void FileWriter::write_char_vec(vector<char> vec)
{
    write_data<size_wg>(vec.size());
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

    // Write the sizes of the standard data types, so that we can throw an error if they're not what we expected. This is extra important for stuff like bool.
    write_data<uint8_t>(sizeof(size_wg));
    write_data<uint8_t>(sizeof(char));
    write_data<uint8_t>(sizeof(short));
    write_data<uint8_t>(sizeof(int));
    write_data<uint8_t>(sizeof(long));
    write_data<uint8_t>(sizeof(long long));
    write_data<uint8_t>(sizeof(float));
    write_data<uint8_t>(sizeof(double));
    write_data<uint8_t>(sizeof(long double));
    write_data<uint8_t>(sizeof(bool));
}

// Writes a string to the file.
void FileWriter::write_string(string str)
{
    size_wg len = str.size();
    write_data<size_wg>(len);
    file_out_.write(str.c_str(), str.size());
}

/* FILEX */
namespace filex {

string exe_dir; // The path to the binary.

// Given a path or filename, combines it with the current executable path and returns the combined, full path.
string game_path(const string_view path) { return merge_paths(get_executable_dir(), path); }

// Loads a text file into an std::string.
string file_to_string(const string_view filename)
{
    string filename_str = string(filename);
    if (!fs::exists(filename)) throw runtime_error("Invalid file: " + filename_str);
    std::ifstream file(filename_str);
    if (!file.is_open()) throw runtime_error("Cannot open file: " + filename_str);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

// Loads a text file into a vector, one string for each line of the file.
vector<string> file_to_vec(const string_view filename, unsigned int flags)
{
    const bool flag_ignore_blank_lines = (flags & FTV_FLAG_IGNORE_BLANK_LINES) == FTV_FLAG_IGNORE_BLANK_LINES;
    const bool flag_ignore_comments = (flags & FTV_FLAG_IGNORE_COMMENTS) == FTV_FLAG_IGNORE_COMMENTS;
    const bool flag_no_strip_newlines = (flags & FTV_FLAG_NO_STRIP_NEWLINES) == FTV_FLAG_NO_STRIP_NEWLINES;

    string filename_str = string{filename};
    if (!fs::exists(filename)) throw runtime_error("Invalid file: " + filename_str);
    vector<string> lines;
    std::ifstream file(filename_str);
    if (!file.is_open()) throw runtime_error("Cannot open file: " + filename_str);

    string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            if (!flag_no_strip_newlines && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
            if (flag_ignore_comments && line.at(0) == '#') continue;
        }
        else if (flag_ignore_blank_lines) continue;
        lines.push_back(line);
    }
    file.close();
    return lines;
}

// Platform-agnostic way to find this binary's runtime directory.
string get_executable_dir()
{
    string result;
#if defined(WESTGATE_TARGET_WINDOWS)
    wchar_t *buf = new wchar_t[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring ws(buf);
    result = string(ws.begin(), ws.end());
    delete[] buf;
#elif defined(WESTGATE_TARGET_LINUX)
    char *buf = new char[1024];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n < 0) throw runtime_error("Could not determine binary path!");
    buf[n] = '\0';
    result = string(buf);
    delete[] buf;
#elif defined(WESTGATE_TARGET_APPLE)
    char *buf = new char[1024];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) throw runtime_error("Could not determine binary path!");
    result = string(buf);
    delete[] buf;
#else
    #error Unsupported/unknown target platform!
#endif
    if (exe_dir.empty()) exe_dir = fs::path(result).parent_path().string();
    return exe_dir;
}

// Merges two path strings together.
string merge_paths(const string_view path_a, const string_view path_b) { return (fs::path(path_a) / path_b).string(); }

} } // filex, westgate namespaces
