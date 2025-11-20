// util/file/fileutils.cpp -- Various utility functions that deal with reading and writing files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>
#include <fstream>
#include <sstream>

#include "util/file/fileutils.hpp"

namespace westgate {
namespace fileutils {

// Loads a text file into an std::string.
std::string file_to_string(const std::string &filename)
{
    if (!std::filesystem::exists(filename)) throw std::runtime_error("Invalid file: " + filename);
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

// Loads a text file into a vector, one string for each line of the file.
std::vector<std::string> file_to_vec(const std::string &filename)
{
    if (!std::filesystem::exists(filename)) throw std::runtime_error("Invalid file: " + filename);
    std::vector<std::string> lines;
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
        lines.push_back(line);
    }
    file.close();
    return lines;
}

} } // fileutils, westgate namespaces
