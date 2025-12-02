// world/area/automap.hpp -- The Automap class automatically generates an ASCII map of the game world to display to the player.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp"

#include <unordered_map>

#include "util/vector3.hpp"

namespace westgate {

class Room; // defined in world/area/room.hpp

class Automap
{
public:
    std::vector<std::string>    generate_map(Room* start_room); // Generates a map centred on the specified coordinate.

private:
    static const Vector3    direction_to_xyz_[10];  // Lookup table for converting Direction enums into X,Y,Z vector directions (e.g. {-1,0,0}).
};

}   // namespace westgate
