// actions/silly.cpp -- Silly commands that serve no real purpose.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "actions/silly.hpp"
#include "core/terminal.hpp"
#include "util/random.hpp"

using westgate::terminal::print;

namespace westgate::actions::silly {

// You are likely to be eaten by a grue.
void magic_word(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    if (rnd::get<int>(1, 50) == 1) print("{K}A hollow voice says, \"Plugh.\"");
    else if (rnd::get<int>(1, 20) == 1) print("{K}Something happens.");
    else print("{K}Nothing happens.");
}

}   // namespace westgate::actions::silly
