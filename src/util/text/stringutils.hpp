// util/text/stringutils.hpp -- Various utility functions that deal with string manipulation/conversion.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

namespace westgate {
namespace stringutils {

std::string                 str_tolower(std::string str);   // Converts a string to lower-case.
std::vector<std::string>    string_explode(std::string str, const std::string &separator = " ");    // String split/explode function.

} } // stringutils, westgate namespaces
