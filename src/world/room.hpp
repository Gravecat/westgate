// world/room.hpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "util/math/vector2.hpp"

namespace westgate {

class Mobile;   // defined in entity/mobile.hpp

class Room {
public:
                        Room(); // Creates a blank Room with default values.
    const std::string&  desc() const;   // Retrieves the description of this Room.
    const std::string&  name() const;   // Retrieves the name of this Room.
    void                set_desc(const std::string& new_desc);  // Sets the description of this Room.
    void                set_name(const std::string& new_name);  // Sets the name of this Room.
    void                set_world_pos(const Vector2 new_pos);   // Sets the new world coordinates for this Room.
    void                transfer(Entity* entity_ptr, Room* room_ptr);   // Transfers a specified Entity from this Room to a target Room.
    const Vector2       world_pos() const;  // Retrieves the world coordinates of this Room.

protected:
    std::vector<std::unique_ptr<Entity>>    entities_;  // Entities that exist within this Room.

private:
    std::string desc_;      // The text description of this Room, as shown to the player.
    std::string name_;      // The name of this Room.
    Vector2     world_pos_; // The X/Y coordinates of this Room in the game's world-space. Negative values are permitted, but {0,0} are forbidden coordinates.
};

}   // namespace westgate
