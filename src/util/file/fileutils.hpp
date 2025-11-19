// util/file/fileutils.hpp -- Various utility functions that deal with creating, deleting, and manipulating files in a platform-agnostic way.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "core/global.hpp"

namespace lom {
namespace fileutils {

void    delete_file(const std::string &filename);   // Deletes a specified file.
bool    directory_exists(const std::string &dir);   // Check if a directory exists.
bool    file_exists(const std::string &file);       // Checks if a file exists.
void    make_dir(const std::string &dir);           // Makes a new directory, if it doesn't already exist.

} } // fileutils, lom namespaces
