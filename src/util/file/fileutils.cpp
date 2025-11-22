// util/file/fileutils.cpp -- Various utility functions that deal with reading and writing files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>
#include <fstream>
#include <sstream>

#include "util/file/fileutils.hpp"

using std::ifstream;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {
namespace fileutils {

// Loads a text file into an std::string.
string file_to_string(const string& filename)
{
    if (!fs::exists(filename)) throw runtime_error("Invalid file: " + filename);
    ifstream file(filename);
    if (!file.is_open()) throw runtime_error("Cannot open file: " + filename);
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

// Loads a text file into a vector, one string for each line of the file.
vector<string> file_to_vec(const string& filename)
{
    if (!fs::exists(filename)) throw runtime_error("Invalid file: " + filename);
    vector<string> lines;
    ifstream file(filename);
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

} } // fileutils, westgate namespaces
