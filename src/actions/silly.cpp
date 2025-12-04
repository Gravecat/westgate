// actions/silly.cpp -- Silly commands that serve no real purpose.

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
