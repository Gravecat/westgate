// actions/world-interaction.hpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

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

#include "parser/parser-macros.hpp"

namespace westgate::actions::world_interaction {

void    look(PARSER_FUNCTION);      // Look around you. Just look around you.
void    travel(PARSER_FUNCTION);    // Travels in a specific direction.
void    open_close(PARSER_FUNCTION);    // Attempts to open or close a door or similar.
void    wait(PARSER_FUNCTION);      // Waits or rests for a specified period of time.

}   // westgate::actions::world_interaction namespace
