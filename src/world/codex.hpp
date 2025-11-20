// world/codex.hpp -- The Codex class handles loading and storing all the static game data (rooms, mobiles, items, etc.) and producing copies as requested.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <memory>

namespace westgate {

class ProcNameGen;  // defined in misc/namegen.hpp

class Codex {
public:
                    Codex();    // Loads the static data from the gorp.k10 datafile.
                    ~Codex();   // Destructor, explicitly frees memory used.
    ProcNameGen&    namegen() const;    // Returns a reference to the procedural name generator object.

private:
    std::unique_ptr<ProcNameGen>    namegen_ptr_;       // Pointer to the procedural name-generator object.
};

Codex&  codex();    // Shortcut instead of using game()->codex()

}   // namespace westgate
