// core/terminal.hpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <climits>
#include <string>

namespace westgate {
namespace terminal {

unsigned int        get_cursor_x(); // Attempts to get the horizontal position of the 'cursor', where output is being printed.
const std::string   get_input();    // Prints a standard cursor and waits for non-zero input from the player.
int                 get_number(int lowest = INT_MIN, int highest = INT_MAX);    // As with get_input(), but requires the user to enter an integer number.
unsigned int        get_width();    // Gets the width of the console window, in characters. If anything goes wrong, it'll return 0.
void                print(const std::string& text = "", bool newline = true);   // Prints a string of text with std::cout, processing ANSI colour tags.
void                set_window_title(const std::string& new_title); // Attempts to set the title of the console window. May not work on all platforms.

} } // namespace terminal, westgate
