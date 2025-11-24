// util/text/stringutils.hpp -- Various utility functions that deal with string manipulation/conversion.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {
namespace stringutils {

constexpr uint8_t CL_MODE_USE_AND = 1;  // Use 'and' for the last entry in comma_list().
constexpr uint8_t CL_MODE_USE_OR =  2;  // Use 'or' for the last entry in comma_list();

std::string comma_list(std::vector<std::string> vec, uint8_t mode = 0);    // Converts a vector to a comma-separated list.
std::string str_tolower(std::string str);   // Converts a string to lower-case.
std::vector<std::string>    string_explode(std::string str, const std::string& separator = " ");    // String split/explode function.
std::string strip_trailing_newlines(std::string str);   // Strips trailing newlines from a given string.

} } // stringutils, westgate namespaces
