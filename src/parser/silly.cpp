// parser/silly.cpp -- Silly commands that serve no real purpose.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"
#include "parser/cheats.hpp"
#include "trailmix/math/random.hpp"

using namespace trailmix;
using westgate::terminal::print;

namespace westgate {
namespace parser {
namespace silly {

// You are likely to be eaten by a grue.
void magic_word(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    if (random::get<int>(1, 50) == 1) print("{K}A hollow voice says, \"Plugh.\"");
    else if (random::get<int>(1, 20) == 1) print("{K}Something happens.");
    else print("{K}Nothing happens.");
}

} } }   // silly, parser, westgate namespaces
