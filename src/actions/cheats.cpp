// actions/cheats.cpp -- Cheating/development/testing commands.

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

#include "core/terminal.hpp"
#include "actions/cheats.hpp"
#include "world/time/time-weather.hpp"
#include "world/world.hpp"

using std::to_string;
using westgate::terminal::print;

namespace westgate::actions::cheats {

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

}   // namespace westgate::actions::cheats
