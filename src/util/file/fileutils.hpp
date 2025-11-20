// util/file/fileutils.hpp -- Various utility functions that deal with reading and writing files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "core/global.hpp"

namespace westgate {
namespace fileutils {

std::string                 file_to_string(const std::string &filename);    // Loads a text file into an std::string.
std::vector<std::string>    file_to_vec(const std::string &filename);       // Loads a text file into a vector, one string for each line of the file.

} } // fileutils, westgate namespaces
