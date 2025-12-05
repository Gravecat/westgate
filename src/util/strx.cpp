// util/strx.cpp -- Various utility functions that deal with manipulating strings.

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

#include <algorithm>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>

#include "3rdparty/murmurhash3/MurmurHash3.h"
#include "util/strx.hpp"

using std::string;
using std::string_view;
using std::to_string;
using std::vector;

namespace westgate {

// Strips all ANSI colour tags like {M} from a string.
string StrX::ansi_strip(string_view str)
{
    string result = string{str};
    while(true)
    {
        size_t found_open = result.find_first_of('{');
        size_t found_closed = result.find_first_of('}', found_open);
        if (found_open == string::npos || found_closed == string::npos) return result;
        result = result.substr(0, found_open) + result.substr(found_closed + 1);
    }
}

// Returns the length of a specified string, not counting the ANSI colour tags like {G} or {kR}.
size_t StrX::ansi_strlen(std::string_view str)
{ return ansi_strip(str).length(); }

// Splits an ANSI-tagged string across multiple lines of text.
vector<string> StrX::ansi_vector_split(string_view str, uint32_t line_length)
{
    string current_line, last_tag;
    vector<string> result, words = string_explode(str, " ");
    uint32_t current_pos = 0;

    while(words.size())
    {
        string word = words.at(0);
        words.erase(words.begin());

        // Determine if any ANSI tags are within this word. If so, record the last tag. Then measure the word length.
        size_t found_open = word.find_last_of('{');
        size_t found_closed = word.find_first_of('}', found_open);
        size_t word_len = 0;
        bool newline_tag = false;
        if (found_open != string::npos && found_closed != string::npos)
        {
            string tag_found = word.substr(found_open, found_closed - found_open + 1);
            if (!tag_found.compare("{nl}")) newline_tag = true;
            else last_tag = tag_found;
            word_len = ansi_strlen(word);
        }
        else word_len = word.size();

        // Decide whether or not we're starting a new line.
        if ((current_pos && (current_pos + word_len >= line_length)) || newline_tag)
        {
            result.push_back(current_line);
            current_line = (newline_tag ? "  " : "") + last_tag + word;
            current_pos = word_len + (newline_tag ? 3 : 1);
        }
        else
        {
            current_line += (current_pos  ? " " : "") + word;
            current_pos += word_len + 1;
        }
    }
    if (current_line.size()) result.push_back(current_line);
    return result;
}

// Only in debug builds, we're gonna add some extra code to detect hash collisions in real-time. Yes, it'll slow performance by a tiny amount, but it's a
// debug build, we're not expecting maximum optimization and speed here.
#ifdef WESTGATE_BUILD_DEBUG
std::map<uint32_t, std::string> StrX::backward_hash_map_;
void StrX::check_hash_collision(string_view str, uint32_t hash)
{
    auto result_b = backward_hash_map_.find(hash);
    if (result_b == backward_hash_map_.end())
    {
        backward_hash_map_.insert({hash, string{str}});
        return;
    }
    if (!result_b->second.compare(str)) return;
   std::cerr << "Hash collision detected! " + string{str} + " and " + result_b->second + " both hash to " + to_string(hash);
}
#endif  // WESTGATE_BUILD_DEBUG

// Converts a vector to a comma-separated list.
string StrX::comma_list(vector<string> vec, uint8_t mode)
{
    string plus = ", ";
    if (mode == CL_MODE_USE_AND) plus = " and ";
    else if (mode == CL_MODE_USE_OR) plus = " or ";
    else if (vec.size() == 2) return vec.at(0) + plus + vec.at(1);

    string str;
    for (unsigned int i = 0; i < vec.size(); i++)
    {
        str += vec.at(i);
        if (i < vec.size() - 1)
        {
            if (i == vec.size() - 2) str += plus;
            else str += ", ";
        }
    }
    return str;
}

// Decodes a compressed string (e.g. 4cab2z becomes ccccabzz).
string StrX::decode_compressed_string(string_view str)
{
    string result, cb = string{str};
    while(cb.size())
    {
        string letter = cb.substr(0, 1);
        cb = cb.substr(1);
        if (letter[0] >= '0' && letter[0] <= '9')
        {
            int number = letter[0] - '0';
            letter = cb.substr(0, 1);
            cb = cb.substr(1);
            while (letter[0] >= '0' && letter[0] <= '9')
            {
                number *= 10;
                number += letter[0] - '0';
                letter = cb.substr(0, 1);
                cb = cb.substr(1);
            }
            result += string(number, letter[0]);
        }
        else result += letter;
    }
    return result;
}

// Find and replace one string with another.
bool StrX::find_and_replace(string& input, string_view to_find, string_view to_replace)
{
    string::size_type pos = 0;
    const string::size_type find_len = to_find.length(), replace_len = to_replace.length();
    if (find_len == 0) return false;
    bool found = false;
    while ((pos = input.find(to_find, pos)) != string::npos)
    {
        found = true;
        input.replace(pos, find_len, to_replace);
        pos += replace_len;
    }
    return found;
}

// 'Flattens' ANSI tags, by erasing redundant tags in the string.
string StrX::flatten_tags(string_view str)
{
    string output, last_tag, to_check = string{str};

    while(true)
    {
        size_t tag_open = to_check.find_first_of('{');
        size_t tag_closed = to_check.find_first_of('}', tag_open);
        if (tag_open == string::npos || tag_closed == string::npos)
        {
            output += to_check;
            break;
        }
        string tag_found = to_check.substr(tag_open + 1, tag_closed - tag_open - 1);
        if (tag_found.compare(last_tag))
        {
            last_tag = tag_found;
            output += to_check.substr(0, tag_closed + 1);
        }
        else output += to_check.substr(0, tag_open);
        to_check = to_check.substr(tag_closed + 1);
    }

    return output;
}

// Converts a float or double to a string.
string StrX::ftos(double num, int precision)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << num;
    return ss.str();
}

// Hashes a string with MurmurHash3.
uint32_t StrX::murmur3(string_view str)
{
    const uint32_t seed = 0x9747b28c;
    uint32_t hash = 0;  // Shouldn't matter, but I don't like uninitialized variables on principle.
    MurmurHash3_x86_32(str.data(), static_cast<int>(str.size()), seed, &hash);

#ifdef WESTGATE_BUILD_DEBUG
    check_hash_collision(str, hash);
#endif

    return hash;
}

// Converts a number (e.g. 123) into a string (e.g. "one hundred and twenty-three").
string StrX::number_to_text(int64_t num)
{
    bool negative = false;
    if (num == 0) return "zero";
    else if (num > 999999999999LL) return "more than nine hundred and ninety-nine billion";
    else if (num < -999999999999LL) return "less than minus nine hundred and ninety-nine billion";
    else if (num < 0)
    {
        negative = true;
        num = -num;
    }

    const string below_twenty[] = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen",
    "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen" };
    const string tens[] = { "", "ten", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety" };
    const string scale[] = { "", " thousand", " million", " billion" };

    auto thousand_chunk = [&below_twenty, &tens](int num) -> string {
        string result;
        if (num >= 100)
        {
            result = below_twenty[num / 100] + " hundred";
            num %= 100;
            if (num) result += " and ";
        }
        if (num >= 20)
        {
            result += tens[num / 10];
            if (num % 10) result += "-" + below_twenty[num % 10];
        }
        else if (num > 0 || result.empty()) result += below_twenty[num];
        return result;
    };

    string output;
    int group = 0;
    while(num > 0 && group < 4)
    {
        int chunk = num % 1000;
        num /= 1000;
        if (chunk)
        {
            string result = thousand_chunk(chunk) + scale[group];
            output = result + (output.empty() ? "" : (group > 0 ? ", " : " ") + output);
        }
        group++;
    }
    return (negative ? "minus " : "") + output;
}

// Allows adding conditional tags to a string in the form of [tag_name:conditional text here] and either including or removing the conditional text depending on
// whether the bool is true or false.
void StrX::process_conditional_tags(string& str, string_view tag, bool active)
{
    string tag_str = string{tag};
    do
    {
        const size_t start = str.find("[" + tag_str);
        const size_t end = str.find("]", start);
        if (start == string::npos || end == string::npos) return;
        if (active)
        {
            const size_t insert_start = start + tag.size() + 2;
            const string insert = str.substr(insert_start, end - insert_start);
            str = str.substr(0, start) + insert + str.substr(end + 1);
        }
        else str = str.substr(0, start) + str.substr(end + 1);
    } while(true);
}

// Converts a string to lower-case.
string StrX::str_tolower(string_view str)
{
    string output = string{str};
    std::transform(output.begin(), output.end(), output.begin(), ::tolower);
    return output;
}

// Converts a string to upper-case.
string StrX::str_toupper(string_view str)
{
    string output = string{str};
    std::transform(output.begin(), output.end(), output.begin(), ::toupper);
    return output;
}

// String split/explode function.
vector<string> StrX::string_explode(string_view str, string_view separator)
{
    vector<string> results;

    string::size_type pos = str.find(separator, 0);
    const size_t pit = separator.length();
    string line = string{str};

    while(pos != string::npos)
    {
        if (pos == 0) results.push_back("");
        else results.push_back(line.substr(0, pos));
        line.erase(0, pos + pit);
        pos = line.find(separator, 0);
    }
    results.push_back(line);

    return results;
}

}   // namespace westgate
