// parser/cheats.cpp -- Cheating/development/testing commands.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"
#include "parser/cheats.hpp"

using std::to_string;
using westgate::terminal::print;

namespace westgate {
namespace parser {
namespace cheats {

// Hashes words into integers.
void hash(PARSER_FUNCTION)
{
    if (words.size() < 2)
    {
        print("{Y}Please specify a word to be hashed.");
        return;
    }
    print("The hashed version of {C}" + words.at(1) + " {w}is {C}" + to_string(words_hashed.at(1)));
}

} } }   // cheats, parser, westgate namespaces
