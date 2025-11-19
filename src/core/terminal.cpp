// core/terminal.cpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"

namespace lom {
namespace terminal {

// Attempts to set the title of the console window. May not work on all platforms.
void set_window_title(std::string new_title)
{
#ifdef LOM_TARGET_WINDOWS
    SetConsoleTitleA(new_title.c_str());
#else
    cout << "\033]2;" << new_title << "\007" << std::flush;
#endif
}

} } // namespace terminal, lom
