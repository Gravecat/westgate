// util/strx.hpp -- Various utility functions that deal with manipulating strings.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#pragma once
#include "core/pch.hpp" // precompiled header

#include <map>

namespace westgate::strx {

constexpr unsigned int  CL_MODE_USE_AND = 1;   // Use 'and' for the last entry in comma_list().
constexpr unsigned int  CL_MODE_USE_OR =  2;   // Use 'or' for the last entry in comma_list();

std::string ansi_strip(const std::string_view str); // Strips all ANSI colour tags like {M} from a string.
size_t      ansi_strlen(const std::string_view str);    // Returns the length of a specified string, not counting the ANSI colour tags like {G}.
                            // Splits an ANSI-tagged string across multiple lines of text.
std::vector<std::string>    ansi_vector_split(const std::string_view str, size_t line_length);
std::string comma_list(std::vector<std::string> vec, unsigned int mode = 0);    // Converts a vector to a comma-separated list.
std::string decode_compressed_string(const std::string_view str);   // Decodes a compressed string (e.g. 4cab2z becomes ccccabzz).
            // Find and replace one string with another.
bool        find_and_replace(std::string& input, const std::string_view to_find, const std::string_view to_replace);
std::string flatten_tags(const std::string_view str);   // 'Flattens' ANSI tags, by erasing redundant tags in the string.
std::string ftos(double num, int precision = 1);    // Converts a float or double to a string.
hash_wg     murmur3(const std::string_view str);    // Hashes a string with MurmurHash3.
std::string number_to_text(int64_t num);    // Converts a number (e.g. 123) into a string (e.g. "one hundred and twenty-three").
            // Allows adding conditional tags to a string in the form of [tag_name:conditional text here] and either including or removing the conditional text
            // depending on whether the bool is true or false.
void        process_conditional_tags(std::string& str, const std::string_view tag, bool active);
std::string str_tolower(const std::string_view str);    // Converts a string to lower-case.
std::string str_toupper(const std::string_view str);    // Converts a string to upper-case.
std::vector<std::string>    string_explode(const std::string_view str, const std::string_view separator = " "); // String split/explode function.

}   // namespace westgate::strx
