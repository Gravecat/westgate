// world/area/region.hpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "world/area/room.hpp"

namespace westgate {

class Region
{
public:
            Region();                   // Creates an empty Region.
            Region(size_t new_size);    // As above, but also calls set_size() to allocate memory.
            ~Region();                  // Destructor, cleans up stored data.
    void    add_room(std::unique_ptr<Room> new_room);   // Adds a new Room to this Region. Must be called with std::move.
    void    load_from_gamedata(const std::string& filename);        // Loads a Region from YAML game data.
    void    load_from_save(int save_slot, unsigned int region_id);  // Loads this Region from a saved game file.
    void    save(int save_slot);        // Saves the Region to a saved game file.
    void    set_size(size_t new_size);  // Reallocates memory for the rooms_ vector, if we know exactly how large it's gonna be.

private:
    static constexpr uint32_t   REGION_SAVE_VERSION =   1;  // The expected version for saving/loading binary game data.
    static constexpr int        REGION_YAML_VERSION =   1;  // The expected version for region YAML data.

    void    rebuild_room_id_map();  // Rebuilds the room ID map, for quickly looking up Rooms by their hashed ID.

    unsigned int    id_;    // The ID of the loaded region file.
    bool            memory_allocated_;  // Have we already called set_size() or specified a size in the constructor?
    std::string     name_;  // The name of this Region.
    std::vector<std::unique_ptr<Room>>  rooms_; // All the Rooms stored within this Region.
    std::map<uint32_t, Room*>   room_ids_;  // An index of all Room IDs, along with pointers to each Room.
};

}   // namespace westgate
