// world/entity/player.cpp -- The Player class defines the player character; it's mostly just a Mobile, but has a few extra player-specific bits.

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

#include "core/game.hpp"
#include "util/filex.hpp"
#include "world/entity/player.hpp"
#include "world/area/room.hpp"

using std::runtime_error;
using std::to_string;

namespace westgate {

// Creates a blank Player, then loads its data from a FileReader.
Player::Player(FileReader* file) : Mobile(file)
{
    region_ = 0;
    set_name("you");
    set_tag(EntityTag::ProperNoun);
    game().set_player(this);

    if (!file) return;

    // Check the save version for this Player.
    const unsigned int save_version = file->read_data<unsigned int>();
    if (save_version != PLAYER_SAVE_VERSION) FileReader::standard_error("Invalid player save version", save_version, PLAYER_SAVE_VERSION);

    // Load the Player's tags, if any.
    const unsigned int tags_tag = file->read_data<unsigned int>();
    if (tags_tag != PLAYER_SAVE_TAGS) FileReader::standard_error("Invalid tag in player save data", tags_tag, PLAYER_SAVE_TAGS);
    size_wg tag_count = file->read_data<size_wg>();
    for (size_wg t = 0; t < tag_count; t++)
        set_player_tag(file->read_data<PlayerTag>());
}

// Clears a PlayerTag from this Player.
void Player::clear_player_tag(PlayerTag the_tag)
{
    if (!(player_tags_.count(the_tag) > 0)) return;
    player_tags_.erase(the_tag);
}

// Clears multiple PlayerTags at the same time.
void Player::clear_player_tags(std::list<PlayerTag> tags_list) { for (auto the_tag : tags_list) clear_player_tag(the_tag); }

// Checks if a PlayerTag is set on this Player.
bool Player::player_tag(PlayerTag the_tag) const { return (player_tags_.count(the_tag) > 0); }

// Checks what Region the Player is currently in.
int Player::region() const { return region_; }

// Saves this Player to a save game file.
void Player::save(FileWriter* file)
{
    Mobile::save(file);
    file->write_data<unsigned int>(PLAYER_SAVE_VERSION);

    // Write the PlayerTags, if any.
    file->write_data<unsigned int>(PLAYER_SAVE_TAGS);
    file->write_data<size_wg>(player_tags_.size());
    for (auto &tag : player_tags_)
        file->write_data<PlayerTag>(tag);
}

// This is a big no-no. We're overriding this method for safety reasons.
void Player::set_parent_entity(Entity* new_entity_parent)
{
    if (new_entity_parent) throw runtime_error("Attempt to set Player to non-null Entity parent!");
    else parent_entity_ = nullptr;
}

// Sets a new Room as the parent of this Player.
void Player::set_parent_room(Room* new_room_parent)
{
    Mobile::set_parent_room(new_room_parent);
    region_ = new_room_parent->region();
    new_room_parent->set_tag(RoomTag::Explored);
}

// Sets a PlayerTag on this Player.
void Player::set_player_tag(PlayerTag the_tag)
{
    if (player_tags_.count(the_tag) > 0) return;
    player_tags_.insert(the_tag);
}

// Sets multiple PlayerTags at the same time.
void Player::set_player_tags(std::list<PlayerTag> tags_list) { for (auto the_tag : tags_list) set_player_tag(the_tag); }

// Toggles a PlayerTag on or off.
void Player::toggle_player_tag(PlayerTag the_tag)
{
    if (!player_tag(the_tag)) set_player_tag(the_tag);
    else clear_player_tag(the_tag);
}

// A shortcut instead of using game().player()
Player& player() { return game().player(); }

}   // namespace westgate
