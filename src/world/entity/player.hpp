// world/entity/player.hpp -- The Player class defines the player character; it's mostly just a Mobile, but has a few extra player-specific bits.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include "world/entity/mobile.hpp"

namespace westgate {

enum class PlayerTag : uint16_t {
    // Tags for features the player has explicitly enabled or disabled.
    AutomapOff =    1,  // The player has disabled the auto-generating minimap.

    // Tags for tutorial messages that will only appear once per game.
    TutorialAutomap =   100,    // The player has seen the automap tutorial message.
};

class Player : public Mobile {
public:
                Player() = delete;  // No default constructor; use nullptr on the constructor below.
                Player(FileReader* file);   // Creates a blank Player, then loads its data from a FileReader.
    void        clear_player_tag(PlayerTag the_tag);    // Clears a PlayerTag from this Player.
    void        clear_player_tags(std::list<PlayerTag> tags_list);  // Clears multiple PlayerTags at the same time.
    bool        player_tag(PlayerTag the_tag) const;    // Checks if a PlayerTag is set on this Player.
    uint32_t    region() const;     // Checks what Region the Player is currently in.
    void        save(FileWriter* file) override;    // Saves this Player to a save game file.
    void        set_parent_entity(Entity* new_entity_parent = nullptr) override;    // This is a big no-no. We're overriding this method for safety reasons.
    void        set_parent_room(Room* new_room_parent = nullptr) override;  // Sets a new Room as the parent of this Player.
    void        set_player_tag(PlayerTag the_tag);  // Sets a PlayerTag on this Player.
    void        set_player_tags(std::list<PlayerTag> tags_list);    // Sets multiple PlayerTags at the same time.
    void        toggle_player_tag(PlayerTag the_tag);   // Toggles a PlayerTag on or off.
    EntityType  type() const override { return EntityType::PLAYER; }    // Self-identifies this Entity's derived class.

private:
    static constexpr uint32_t   PLAYER_SAVE_VERSION =   2;  // The expected version for saving/loading binary game data.

    // Identifiers for blocks of data in the save file, used to quickly catch errors when loading old or invalid data.
    static constexpr uint32_t   PLAYER_SAVE_TAGS =      1;

    unsigned int    region_;    // The current Region the Player is in.
    std::set<PlayerTag> player_tags_;   // Any and all PlayerTags on the Player.
};

Player& player();   // A shortcut instead of using game().player()

}   // namespace westgate
