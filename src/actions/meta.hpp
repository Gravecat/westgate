// actions/meta.hpp -- "Meta" commands, i.e. commands that interact with the game itself, such as saving or quitting the game.

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

namespace westgate::actions::meta {

void    automap(PARSER_FUNCTION);   // Toggles the automap on or off.
void    quit(PARSER_FUNCTION);      // Quits the game.
void    save(PARSER_FUNCTION);      // Saves the game.

}   // namespace westgate::actions::meta
