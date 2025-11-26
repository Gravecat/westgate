// src/parser/parser.hpp -- The parser processes input from the player and converts it into commands in the game world.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {

enum class Direction : uint8_t; // defined in world/area/room.hpp

namespace parser {

Direction   parse_direction(uint32_t hash); // Parses a hashed string into a Direction enum.
int32_t     parse_number(const std::string &num);   // Attempts to parse a string into a number; invalid results are set to INT_MIN.
void        process_input(const std::string& input);    // Processes input from the player.
bool        yes_no();   // Displays a yes/no prompt for the player, returns their choice.

} } // parser, westgate namespaces
