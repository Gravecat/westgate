// core/terminal.hpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <string>

namespace westgate {
namespace terminal {

unsigned int    get_width();    // Gets the width of the console window, in characters.
void            print(const std::string& text); // Prints a string of text with std::cout, processing ANSI colour tags.
void            set_window_title(const std::string& new_title); // Attempts to set the title of the console window. May not work on all platforms.

} } // namespace terminal, westgate
