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

namespace westgate {

class StrX {
public:
    static constexpr uint8_t CL_MODE_USE_AND = 1;   // Use 'and' for the last entry in comma_list().
    static constexpr uint8_t CL_MODE_USE_OR =  2;   // Use 'or' for the last entry in comma_list();

    static std::string  ansi_strip(std::string_view str);   // Strips all ANSI colour tags like {M} from a string.
    static size_t       ansi_strlen(std::string_view str);  // Returns the length of a specified string, not counting the ANSI colour tags like {G} or {kR}.
                                    // Splits an ANSI-tagged string across multiple lines of text.
    static std::vector<std::string> ansi_vector_split(std::string_view str, uint32_t line_length);
    static std::string  comma_list(std::vector<std::string> vec, uint8_t mode = 0);    // Converts a vector to a comma-separated list.
    static std::string  decode_compressed_string(std::string_view str); // Decodes a compressed string (e.g. 4cab2z becomes ccccabzz).
    // Find and replace one string with another.
    static bool         find_and_replace(std::string& input, std::string_view to_find, std::string_view to_replace);
    static std::string  flatten_tags(std::string_view str);     // 'Flattens' ANSI tags, by erasing redundant tags in the string.
    static std::string  ftos(double num, int precision = 1);    // Converts a float or double to a string.
    static uint32_t     murmur3(std::string_view str);  // Hashes a string with MurmurHash3.
    static std::string  number_to_text(int64_t num);    // Converts a number (e.g. 123) into a string (e.g. "one hundred and twenty-three").
    // Allows adding conditional tags to a string in the form of [tag_name:conditional text here] and either including or removing the conditional text
    // depending on whether the bool is true or false.
    static void         process_conditional_tags(std::string& str, std::string_view tag, bool active);
    static std::string  str_tolower(std::string_view str);  // Converts a string to lower-case.
    static std::string  str_toupper(std::string_view str);  // Converts a string to upper-case.
    static std::vector<std::string> string_explode(std::string_view str, std::string_view separator = " "); // String split/explode function.

private:
// Only in debug builds, we're gonna add some extra code to detect hash collisions in real-time. Yes, it'll slow performance by a tiny amount, but it's a
// debug build, we're not expecting maximum optimization and speed here.
#ifdef WESTGATE_BUILD_DEBUG
    static void check_hash_collision(std::string_view str, uint32_t hash);
    static std::map<uint32_t, std::string>  backward_hash_map_;
#endif
};

}   // namespace westgate
