// world/world.hpp -- The World class handles storing and managing the data for the game world (rooms, items, mobiles, etc.) as well as handling copies of
// templatable things like items and monsters.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <unordered_map>

#ifdef WESTGATE_BUILD_DEBUG
#include "trailmix/math/vector3.hpp"
#include <set>
#endif

namespace westgate {

class Automap;      // defined in world/area/automap.hpp
class ProcNameGen;  // defined in util/text/namegen.hpp
class Region;       // defined in world/area/region.hpp
class Room;         // defined in world/area/room.hpp
class TimeWeather;  // defined in world/time-weather.hpp

class World {
public:
                    World();    // Sets up the World object and loads static data into memory.
                    ~World();   // Destructor, explicitly frees memory used.
    void            add_room_to_region(uint32_t room_id, uint32_t region_id);   // Updates the room_regions_ map to keep track of what Region each Room is in.
    Automap&        automap() const;    // Returns a reference to the automap object.
    void            create_region_saves(int save_slot); // Loads region data from YAML, and saves it as a new save file in the specified slot.
    Room*           find_room(const std::string& id, uint32_t region_id);   // Attempts to find a room by its string ID.
    Room*           find_room(uint32_t id, uint32_t region_id);             // Attempts to find a room by its hashed ID.
    Room*           find_room(uint32_t id);     // As above, but doesn't specify Region ID. This is more computationally expensive.
    uint32_t        find_room_region(uint32_t id) const;    // Attempts to find the Region that a specified Room belongs to.
    Region*         load_region(uint32_t id);   // Specifies a Region to be loaded into memory.
    ProcNameGen&    namegen() const;        // Returns a reference to the procedural name generator object.
                    // Opens/closes a door, without checking for locks/etc., without printing any messages.
    void            open_close_no_checks(Room* room, Direction dir, bool open);
    void            save(int save_slot);    // Saves the game! Should only be called via Game::save().
    TimeWeather&    time_weather() const;   // Returns a reference to the time/weather manager object.
    void            unload_region(uint32_t id); // Removes a Region from memory, saving it first.

#ifdef WESTGATE_BUILD_DEBUG
    void            mark_room_coords_used(trailmix::math::Vector3 coords);  // When in debug mode, mark room coordinates as used, to track overlaps.
#endif

private:
    std::unique_ptr<Automap>        automap_ptr_;   // Pointer to the automapper object.
    std::unique_ptr<ProcNameGen>    namegen_ptr_;   // Pointer to the procedural name-generator object.
    std::unordered_map<uint32_t, std::unique_ptr<Region>>   regions_;   // The Regions currently loaded into memory.
    std::unordered_map<uint32_t, uint32_t>  room_regions_;  // Lookup table to determine which Region each Room is located in.
    std::unique_ptr<TimeWeather>    time_weather_ptr_;  // Pointer to the time/weather manager object.

#ifdef WESTGATE_BUILD_DEBUG
    std::set<trailmix::math::Vector3>   room_coords_used_;  // When in debug mode, this keeps track of Room coordinates, so we can track overlaps.
#endif
};

World&  world();    // Shortcut instead of using game()->world()

}   // namespace westgate
