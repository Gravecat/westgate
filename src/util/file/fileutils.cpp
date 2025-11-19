// util/file/fileutils.cpp -- Various utility functions that deal with creating, deleting, and manipulating files in a platform-agnostic way.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "util/file/fileutils.hpp"

namespace lom {
namespace fileutils {

// Deletes a specified file. Simple enough, but we'll keep this function around in case there's any platform-specific weirdness that needs to be worked in.
void delete_file(const std::filesystem::path& filename) { std::filesystem::remove(filename.c_str()); }

// Check if a directory exists.
bool directory_exists(const std::filesystem::path& dir) { return std::filesystem::exists(dir) && std::filesystem::is_directory(dir); }

// Checks if a file exists.
bool file_exists(const std::filesystem::path& file) { return std::filesystem::exists(file) && !std::filesystem::is_directory(file); }

// Makes a new directory, if it doesn't already exist.
void make_dir(const std::filesystem::path& dir) { if (!std::filesystem::exists(dir)) std::filesystem::create_directory(dir); }

} } // fileutils, lom namespaces
