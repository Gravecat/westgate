// util/file/fileutils.cpp -- Various utility functions that deal with reading and writing files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "util/file/fileutils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace lom {
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

} } // fileutils, lom namespaces
