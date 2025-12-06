// src/parser/parser.hpp -- The parser processes input from the player and converts it into commands in the game world.

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
#include "core/pch.hpp" // Precompiled header

namespace westgate {

enum class Direction : unsigned char;   // defined in world/area/room.hpp

namespace parser {

Direction   parse_direction(uint32_t hash); // Parses a hashed string into a Direction enum.
int32_t     parse_number(const std::string_view num);   // Attempts to parse a string into a number; invalid results are set to INT_MIN.
void        process_input(const std::string_view input);    // Processes input from the player.
bool        yes_no();   // Displays a yes/no prompt for the player, returns their choice.

} } // parser, westgate namespaces
