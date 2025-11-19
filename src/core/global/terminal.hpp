// core/global/terminal.hpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// rang library, used to add colour to the terminal.
#include "3rdparty/rang/rang.hpp"

using namespace rang;

#ifdef EOL
#error EOL definition already exists!
#else
#define EOL rang::style::reset << std::endl
#endif

namespace lom {
namespace terminal {

void    set_window_title(std::string new_title);    // Attempts to set the title of the console window. May not work on all platforms.

} } // namespace terminal, lom
