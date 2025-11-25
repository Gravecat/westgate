// util/text/stringutils.hpp -- Various utility functions that deal with string manipulation/conversion.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {
namespace stringutils {

static constexpr uint8_t CL_MODE_USE_AND = 1;   // Use 'and' for the last entry in comma_list().
static constexpr uint8_t CL_MODE_USE_OR =  2;   // Use 'or' for the last entry in comma_list();

std::string ansi_strip(const std::string &str);     // Strips all ANSI colour tags like {M} from a string.
size_t      ansi_strlen(const std::string &str);    // Returns the length of a specified string, not counting the ANSI colour tags like {G} or {kR}.
std::vector<std::string>    ansi_vector_split(const std::string &str, uint32_t line_length);    // Splits an ANSI-tagged string across multiple lines of text.
std::string comma_list(std::vector<std::string> vec, uint8_t mode = 0);    // Converts a vector to a comma-separated list.
std::string flatten_tags(const std::string &str);   // 'Flattens' ANSI tags, by erasing redundant tags in the string.
std::string number_to_text(int64_t num);    // Converts a number (e.g. 123) into a string (e.g. "one hundred and twenty-three").
std::string str_tolower(std::string str);   // Converts a string to lower-case.
std::vector<std::string>    string_explode(std::string str, const std::string& separator = " ");    // String split/explode function.
std::string strip_trailing_newlines(std::string str);   // Strips trailing newlines from a given string.

} } // stringutils, westgate namespaces
