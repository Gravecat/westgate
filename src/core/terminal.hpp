// core/terminal.hpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/global.hpp"

#include <iostream> // std::cout and associated functionality

// rang library, used to add colour to the terminal.
#include "3rdparty/rang/rang.hpp"

using std::cout;
using std::endl;
using namespace rang;

namespace lom {
namespace terminal {

void    set_window_title(std::string new_title);    // Attempts to set the title of the console window. May not work on all platforms.

} } // namespace terminal, lom
