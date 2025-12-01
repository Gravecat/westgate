// world/area/region.hpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <unordered_map>

#include "world/area/room.hpp"

namespace westgate {

class Region
{
public:
    static constexpr uint32_t   REGION_DELTA_ROOM =         1;  // The delta tag to indicate room data is following.
    static constexpr uint32_t   REGION_DELTA_ROOMS_END =    2;  // The delta tag to indicate the end of the room data.

                Region();                       // Creates an empty Region.
                ~Region();                      // Destructor, cleans up stored data.
    Room*       find_room(const std::string& id) const; // Attempts to find a room by its string ID.
    Room*       find_room(uint32_t id) const;   // Attempts to find a room by its hashed ID.
    uint32_t    id() const;                     // Retrieves this Region's unique ID.
    void        load(int save_slot, uint32_t region_id);    // Loads this Region's YAML data, then applies delta changes from saved game binary data.
    void        load_from_gamedata(const std::string& filename, bool update_world = false); // Loads a Region from YAML game data.
    void        save_delta(int save_slot, bool no_changes = false); // Saves only the changes to this Region in a save file.

private:
    void        load_delta(int save_slot);  // Loads delta changes from a saved game file.

    static constexpr uint32_t   REGION_SAVE_VERSION =       3;  // The expected version for saving/loading binary game data.
    static constexpr uint32_t   REGION_YAML_VERSION =       4;  // The expected version for region YAML data.

    uint32_t    id_;    // The ID of the loaded region file.
    std::string name_;  // The name of this Region.
    std::unordered_map<uint32_t, std::unique_ptr<Room>> rooms_; // All the Rooms stored within this Region.
};

}   // namespace westgate
