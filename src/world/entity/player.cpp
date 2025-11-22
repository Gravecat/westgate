// world/entity/player.cpp -- The Player class defines the player character; it's mostly just a Mobile, but has a few extra player-specific bits.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/game.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "world/entity/player.hpp"

namespace westgate {

// Creates a blank Player, then loads its data from a FileReader.
Player::Player(FileReader* file) : Mobile(file)
{
    set_name("you");
    region_ = room_ = 0;
    game().set_player(this);

    if (!file) return;
    // load file data here
}

// Saves this Player to a save game file.
void Player::save(FileWriter* file) { Mobile::save(file); }

// A shortcut instead of using game().player()
const Player* player() { return game().player(); }

}   // namespace westgate
