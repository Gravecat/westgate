// world/world.hpp -- The World class handles storing and managing the data for the game world (rooms, items, mobiles, etc.) as well as handling copies of
// templatable things like items and monsters.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <unordered_map>

#ifdef WESTGATE_BUILD_DEBUG
#include <set>
#endif

namespace westgate {

class Automap;      // defined in world/area/automap.hpp
class Mobile;       // defined in world/entity/mobile.hpp
class ProcNameGen;  // defined in util/text/namegen.hpp
class Region;       // defined in world/area/region.hpp
class Room;         // defined in world/area/room.hpp
class TimeWeather;  // defined in world/time-weather.hpp
enum class Direction : uint8_t; // defined in world/area/area.hpp

class World {
public:
    enum class OpenCloseLockUnlock : uint8_t { OPEN, CLOSE, LOCK, UNLOCK };

                    World();    // Sets up the World object and loads static data into memory.
                    ~World();   // Destructor, explicitly frees memory used.
    void            add_room_to_region(uint32_t room_id, int region_id);    // Updates the room_regions_ map to keep track of what Region each Room is in.
    Automap&        automap() const;    // Returns a reference to the automap object.
    void            create_region_saves(int save_slot); // Loads region data from YAML, and saves it as a new save file in the specified slot.
    Room*           find_room(const std::string_view id, int region_id);    // Attempts to find a room by its string ID.
    Room*           find_room(uint32_t id, int region_id);  // Attempts to find a room by its hashed ID.
    Room*           find_room(uint32_t id);     // As above, but doesn't specify Region ID. This is more computationally expensive.
    int             find_room_region(uint32_t id) const;    // Attempts to find the Region that a specified Room belongs to.
    Region*         load_region(int id);    // Specifies a Region to be loaded into memory.
    ProcNameGen&    namegen() const;        // Returns a reference to the procedural name generator object.
                    // Opens/closes/locks/unlocks a door, without checking for locks/etc. The checks should be done in player commands or Mobile AI.
    void            open_close_lock_unlock_no_checks(Room* room, Direction dir, OpenCloseLockUnlock type, Mobile* actor);
    void            save(int save_slot);    // Saves the game! Should only be called via Game::save().
    TimeWeather&    time_weather() const;   // Returns a reference to the time/weather manager object.
    void            unload_region(int id);  // Removes a Region from memory, saving it first.

#ifdef WESTGATE_BUILD_DEBUG
    void            debug_mark_room(const std::string_view room_name);  // When in debug mode, mark name hashes as used, to track overlaps.
#endif

private:
    std::unique_ptr<Automap>        automap_ptr_;   // Pointer to the automapper object.
    std::unique_ptr<ProcNameGen>    namegen_ptr_;   // Pointer to the procedural name-generator object.
    std::unordered_map<int, std::unique_ptr<Region>>    regions_;   // The Regions currently loaded into memory.
    std::unordered_map<uint32_t, int>   room_regions_;  // Lookup table to determine which Region each Room is located in.
    std::unique_ptr<TimeWeather>    time_weather_ptr_;  // Pointer to the time/weather manager object.

#ifdef WESTGATE_BUILD_DEBUG
    std::set<uint32_t>  room_name_hashes_used_; // When in debug mode, keeps track of which room names have been used; again, for overlap tracking.
#endif
};

World&  world();    // Shortcut instead of using game()->world()

}   // namespace westgate
