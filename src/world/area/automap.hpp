// world/area/automap.hpp -- The Automap class automatically generates an ASCII map of the game world to display to the player.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp"

#include <unordered_map>

#include "util/math/vector3.hpp"

namespace westgate {

class Room; // defined in world/area/room.hpp

class Automap
{
public:
    void    add_room_vec(uint32_t room_id, Vector3 vec);        // Adds a room to the vector coordinate cache.
    std::vector<std::string>    generate_map(const Room* start_room);   // Generates a map centred on the specified coordinate.

private:
    Room*   find_room(Vector3 pos); // Retrieves a Room pointer for a specified coordinate in the world, or nullptr if it can't be found.

    std::unordered_map<Vector3, uint32_t>   room_vecs_; // Lookup map to locate Room IDs by vector coordinates.
};

}   // namespace westgate
