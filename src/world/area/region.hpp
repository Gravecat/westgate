// world/area/region.hpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <memory>
#include <vector>

#include "world/area/room.hpp"

namespace westgate {

class Region
{
public:
            Region();                   // Creates an empty Region.
            Region(size_t new_size);    // As above, but also calls set_size() to allocate memory.
    void    add_room(std::unique_ptr<Room> new_room);   // Adds a new Room to this Region. Must be called with std::move.
    void    set_size(size_t new_size);  // Reallocates memory for the rooms_ vector, if we know exactly how large it's gonna be.

private:
    bool    memory_allocated_ = false;  // Have we already called set_size() or specified a size in the constructor?
    std::vector<std::unique_ptr<Room>>  rooms_; // All the Rooms stored within this Region.
};

}   // namespace westgate
