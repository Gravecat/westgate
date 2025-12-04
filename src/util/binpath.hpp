// util/binpath.hpp -- Platform-specific code for determining the path of the binary at runtime.
// Originally based on code by Atul R on Stack Overflow, licensed under CC BY-SA 4.0:
// https://stackoverflow.com/questions/1528298/get-path-of-executable
// Modified by Raine Simmons. This adapted version is licensed under the GNU GPL v3.

// Copyright (c) 2025 Raine "Gravecat" Simmons
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "core/pch.hpp" // precompiled header

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
