// world/entity/player.hpp -- The Player class defines the player character; it's mostly just a Mobile, but has a few extra player-specific bits.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>

#include "world/entity/mobile.hpp"

namespace westgate {

class Player : public Mobile {
public:
                Player() = delete;  // No default constructor; use nullptr on the constructor below.
                Player(FileReader* file);   // Creates a blank Player, then loads its data from a FileReader.
    void        save(FileWriter* file) override;    // Saves this Player to a save game file.
    EntityType  type() const override { return EntityType::PLAYER; }    // Self-identifies this Entity's derived class.

    unsigned int    region_;    // The current Region the Player is in.
    uint32_t        room_;      // The Room ID the Player is in.    
};

Player& player();   // A shortcut instead of using game().player()

}   // namespace westgate
