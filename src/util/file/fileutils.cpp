// util/file/fileutils.cpp -- Various utility functions that deal with creating, deleting, and manipulating files in a platform-agnostic way.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <sys/stat.h>   // stat(), mkdir()
#include <unistd.h>     // unlink()

#include "util/file/fileutils.hpp"

namespace lom {
namespace fileutils {

// Deletes a specified file. Simple enough, but we'll keep this function around in case there's any platform-specific weirdness that needs to be worked in.
void delete_file(const std::string &filename) { unlink(filename.c_str()); }

// Check if a directory exists.
bool directory_exists(const std::string &dir)
{
    struct stat info;
    if (stat(dir.c_str(), &info) != 0) return false;
    if (info.st_mode & S_IFDIR) return true;
    return false;
}

// Checks if a file exists.
bool file_exists(const std::string &file)
{
    struct stat info;
    return (stat(file.c_str(), &info) == 0);
}

// Makes a new directory, if it doesn't already exist.
void make_dir(const std::string &dir)
{
    if (directory_exists(dir) || file_exists(dir)) return;

#ifdef LOM_TARGET_WINDOWS
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif
}

} } // fileutils, lom namespaces
