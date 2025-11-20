// world/area/region.cpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "world/area/region.hpp"

namespace westgate {

// Creates an empty Region.
Region::Region() : memory_allocated_(false) { }

// As above, but also calls set_size() to allocate memory.
Region::Region(size_t new_size) : memory_allocated_(false) { set_size(new_size); }

// Adds a new Room to this Region. Must be called with std::move.
void Region::add_room(std::unique_ptr<Room> new_room)
{
    if (!new_room) core().nonfatal("Attempted to add null room to region.", Core::CORE_ERROR);
    else rooms_.push_back(std::move(new_room));
}

// Reallocates memory for the rooms_ vector, if we know exactly how large it's gonna be.
void Region::set_size(size_t new_size)
{
    if (memory_allocated_) core().nonfatal("Attempting to resize a Region that already has a defined size.", Core::CORE_ERROR);
    rooms_.reserve(new_size);
    memory_allocated_ = true;
}

}   // namespace westgate
