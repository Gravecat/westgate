// world/area/automap.hpp -- The Automap class automatically generates an ASCII map of the game world to display to the player.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp"

#include <unordered_map>

#include "trailmix/math/vector3.hpp"

namespace westgate {

class Room; // defined in world/area/room.hpp

class Automap
{
public:
    void    add_room_vec(uint32_t room_id, trailmix::math::Vector3 vec);    // Adds a room to the vector coordinate cache.
    std::vector<std::string>    generate_map(Room* start_room); // Generates a map centred on the specified coordinate.

private:
    Room*   find_room(trailmix::math::Vector3 pos); // Retrieves a Room pointer for a specified coordinate in the world, or nullptr if it can't be found.

    static const trailmix::math::Vector3    direction_to_xyz_[10];  // Lookup table for converting Direction enums into X,Y,Z vector directions (e.g. {-1,0,0}).

    std::unordered_map<trailmix::math::Vector3, uint32_t> room_vecs_;   // Lookup map to locate Room IDs by vector coordinates.
};

}   // namespace westgate
