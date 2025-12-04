// core/terminal.hpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

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
namespace terminal {

unsigned int        get_cursor_x(); // Attempts to get the horizontal position of the 'cursor', where output is being printed.
const std::string   get_input();    // Prints a standard cursor and waits for non-zero input from the player.
                    // As with get_input(), but requires the user to enter an integer number. If yes_no is true, it allows yes/no to translate to 1/0.
int                 get_number(int lowest = INT_MIN, int highest = INT_MAX, bool yes_no = false);
unsigned int        get_width();    // Gets the width of the console window, in characters. If anything goes wrong, it'll return 0.
void                print(const std::string& text = "", bool newline = true);   // Prints a string of text with std::cout, processing ANSI colour tags.
void                set_window_title(const std::string& new_title); // Attempts to set the title of the console window. May not work on all platforms.

} } // namespace terminal, westgate
