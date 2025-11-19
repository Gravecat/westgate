// core/core.hpp -- Main program entry, initialization and cleanup routines, along with pointers to the key subsystems of the game.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "core/global.hpp"

namespace lom {

class Core {
public:
    static Core&    core(); // Returns a reference to the singleton Core object.
    void            init_core(std::vector<std::string> parameters);   // Sets up the core game classes and data, and the terminal subsystem.
    void            destroy_core(int exit_code);    // Destroys the singleton Core object and ends execution.

private:
            Core();     // Constructor, sets up the Core object.
    void    cleanup();  // Attempts to gracefully clean up memory and subsystems.
    void    great_googly_moogly_its_all_gone_to_shit(); // Applies the most powerful possible method to kill the process, in event of emergency.

};  // class Core

Core&   core(); // A shortcut to using Core::core().

}   // namespace lom
