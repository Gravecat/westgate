// util/strx.hpp -- Various utility functions that deal with manipulating strings.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <map>
#include <string>
#include <vector>

namespace westgate {

class StrX {
public:
    static constexpr uint8_t CL_MODE_USE_AND = 1;   // Use 'and' for the last entry in comma_list().
    static constexpr uint8_t CL_MODE_USE_OR =  2;   // Use 'or' for the last entry in comma_list();

    static std::string  ansi_strip(const std::string& str);     // Strips all ANSI colour tags like {M} from a string.
    static size_t       ansi_strlen(const std::string& str);    // Returns the length of a specified string, not counting the ANSI colour tags like {G} or {kR}.
    // Splits an ANSI-tagged string across multiple lines of text.
    static std::vector<std::string> ansi_vector_split(const std::string& str, uint32_t line_length);
    static std::string  comma_list(std::vector<std::string> vec, uint8_t mode = 0);    // Converts a vector to a comma-separated list.
    static std::string  decode_compressed_string(std::string cb);   // Decodes a compressed string (e.g. 4cab2z becomes ccccabzz).
    // Find and replace one string with another.
    static bool         find_and_replace(std::string& input, const std::string& to_find, const std::string& to_replace);
    static std::string  flatten_tags(const std::string& str);   // 'Flattens' ANSI tags, by erasing redundant tags in the string.
    static std::string  ftos(double num, int precision = 1);    // Converts a float or double to a string.
    static uint32_t     murmur3(const std::string& str);    // Hashes a string with MurmurHash3.
    static std::string  number_to_text(int64_t num);    // Converts a number (e.g. 123) into a string (e.g. "one hundred and twenty-three").
    // Allows adding conditional tags to a string in the form of [tag_name:conditional text here] and either including or removing the conditional text
    //  depending on whether the bool is true or false.
    static void         process_conditional_tags(std::string& str, const std::string& tag, bool active);
    static std::string  str_tolower(std::string str);   // Converts a string to lower-case.
    static std::string  str_toupper(std::string str);   // Converts a string to upper-case.
    static std::vector<std::string> string_explode(std::string str, const std::string& separator = " ");    // String split/explode function.

private:
// Only in debug builds, we're gonna add some extra code to detect hash collisions in real-time. Yes, it'll slow performance by a tiny amount, but it's a
// debug build, we're not expecting maximum optimization and speed here.
#ifdef WESTGATE_BUILD_DEBUG
    static void check_hash_collision(const std::string& str, uint32_t hash);
    static std::map<uint32_t, std::string>  backward_hash_map_;
#endif
};

}   // namespace westgate
