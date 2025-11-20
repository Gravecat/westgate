// util/file/binpath.hpp -- Some very nasty platform-specific code for determining the path of the binary file at runtime.
// Based on and modified from code I found on StackOverflow: https://stackoverflow.com/questions/1528298/get-path-of-executable

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <string>

namespace westgate {

class BinPath {
public:
    // Given a path or filename, combines it with the current executable path and returns the combined, full path. At least, that's the hope.
    static std::string  game_path(const std::string &path);
    static std::string  merge_paths(const std::string &pathA, const std::string &pathB);    // Merges two path strings together.

private:
    static std::string  exe_dir;    // The path to the binary.

    static std::string  get_executable_dir();   // Platform-agnostic way to find this binary's runtime directory.
    static std::string  get_executable_path();  // Platform-agnostic way to find this binary's runtime path.
};

}   // namespace westgate
