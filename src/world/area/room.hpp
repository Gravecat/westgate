// world/area/room.hpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "world/entity/entity.hpp"

namespace westgate {

class Room {
public:
                        Room(); // Creates a blank Room with default values.
    const std::string&  desc() const;   // Retrieves the description of this Room.
    const std::string&  name() const;   // Retrieves the name of this Room.
    void                set_desc(const std::string& new_desc);  // Sets the description of this Room.
    void                set_name(const std::string& new_name);  // Sets the name of this Room.
    void                transfer(Entity* entity_ptr, Room* room_ptr);   // Transfers a specified Entity from this Room to a target Room.

protected:
    std::vector<std::unique_ptr<Entity>>    entities_;  // Entities that exist within this Room.

private:
    std::string desc_;      // The text description of this Room, as shown to the player.
    std::string name_;      // The name of this Room.
};

}   // namespace westgate
