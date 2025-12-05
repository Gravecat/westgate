// world/world.cpp -- The World class handles storing and managing the data for the game world (rooms, items, mobiles, etc.) as well as handling copies of
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

#include <filesystem>

#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "util/filex.hpp"
#include "util/namegen.hpp"
#include "util/strx.hpp"
#include "util/timer.hpp"
#include "world/area/automap.hpp"
#include "world/area/region.hpp"
#include "world/entity/player.hpp"
#include "world/time/time-weather.hpp"
#include "world/world.hpp"

using std::make_unique;
using std::runtime_error;
using std::string;
using std::string_view;
using std::to_string;
using std::unique_ptr;
using std::vector;
using westgate::terminal::print;
namespace fs = std::filesystem;

namespace westgate {

// Sets up the World object and loads static data into memory.
World::World() : automap_ptr_(make_unique<Automap>()), namegen_ptr_(make_unique<ProcNameGen>()), time_weather_ptr_(make_unique<TimeWeather>())
{
    Timer init_world;
    core().log("Loading static data into memory.");
    namegen_ptr_->load_namelists();
#ifdef WESTGATE_BUILD_DEBUG
    core().log("Static data loaded in " + StrX::ftos(init_world.elapsed() / 1000.0f, 3) + " seconds.");
#endif
}

// Destructor, explicitly frees memory used.
World::~World()
{
    automap_ptr_.reset(nullptr);
    namegen_ptr_.reset(nullptr);
    time_weather_ptr_.reset(nullptr);
}

// Updates the room_regions_ map to keep track of what Region each Room is in.
void World::add_room_to_region(uint32_t room_id, int region_id)
{ room_regions_.insert({room_id, region_id}); }

// Returns a reference to the automap object.
Automap& World::automap() const
{
    if (!automap_ptr_) throw runtime_error("Attempt to access null Automap object!");
    return *automap_ptr_;
}

// Loads region data from YAML, and saves it as a new save file in the specified slot.
void World::create_region_saves(int save_slot)
{
    // This can be replaced with something better later.
    print("{c}Generating game world from static data...");

    // Create a game saves folder, if one doesn't already exist.
    const fs::path userdata_saves_path = FileX::game_path("userdata/saves");
    if (!fs::is_directory(userdata_saves_path)) fs::create_directory(userdata_saves_path);

    // This early in development, we're gonna just delete the save folder each time. It'll become more permanent later.
    const fs::path save_dir = FileX::merge_paths(userdata_saves_path.string(), to_string(save_slot));
    fs::remove_all(save_dir);
    fs::create_directory(save_dir);

    // Determine how many region files are in the game's data files.
    const fs::path regions_folder = core().datafile("world/regions");
    vector<fs::path> regions;
    for (const auto& file : fs::directory_iterator(regions_folder))
        if (file.is_regular_file()) regions.push_back(file.path().filename());

    // One at a time, load each region into memory, and save an empty delta changes binary file for it.
    for (size_t i = 0; i < regions.size(); i++)
    {
        fs::path region_file = regions.at(i);
        unique_ptr<Region> new_region = make_unique<Region>();
        new_region->load_from_gamedata(region_file.string());
        new_region->save_delta(save_slot, true);
    }
}

#ifdef WESTGATE_BUILD_DEBUG
// When in debug mode, mark name hashes as used, to track overlaps.
void World::debug_mark_room(string_view room_name)
{
    const uint32_t room_name_hash = StrX::murmur3(room_name);
    if (room_name_hashes_used_.count(room_name_hash) > 0) throw runtime_error("Room name hash collision detected: " + string{room_name});
    room_name_hashes_used_.insert(room_name_hash);
}
#endif

// Attempts to find a room by its string ID.
Room* World::find_room(string_view id, int region_id)
{ return find_room(StrX::murmur3(id), region_id); }

// Attempts to find a room by its hashed ID.
Room* World::find_room(uint32_t id, int region_id)
{
    auto region = regions_.find(region_id);
    if (region == regions_.end())
    {
        // If the Region isn't currently loaded, then load it into memory.
        return load_region(region_id)->find_room(id);
    }
    return region->second->find_room(id);
}

// As above, but doesn't specify Region ID. This is more computationally expensive.
Room* World::find_room(uint32_t id)
{ return find_room(id, find_room_region(id)); }

// Attempts to find the Region that a specified Room belongs to.
int World::find_room_region(uint32_t id) const
{
    auto result = room_regions_.find(id);
    if (result == room_regions_.end()) throw runtime_error("Unable to locate room " + to_string(id));
    return result->second;
}

// Specifies a Region to be loaded into memory.
Region* World::load_region(int id)
{
    auto region = regions_.find(id);
    if (region != regions_.end()) return region->second.get();  // It's already loaded.

    auto new_region = std::make_unique<Region>();
    new_region->load(game().save_slot(), id);
    Region* region_ptr = new_region.get();
    regions_.insert({id, std::move(new_region)});
    return region_ptr;
}

// Returns a reference to the procedural name generator object.
ProcNameGen& World::namegen() const
{
    if (!namegen_ptr_) throw runtime_error("Attempt to access null ProcNameGen object!");
    return *namegen_ptr_;
}

// Opens/closes/locks/unlocks a door, without checking for locks/etc. The checks should be done in player commands or Mobile AI.
void World::open_close_lock_unlock_no_checks(Room* room, Direction dir, OpenCloseLockUnlock type, Mobile* actor)
{
    if (!room) throw runtime_error("Attempt to open/close/lock/unlock door with null room pointer!");
    if (!room->has_exit(dir)) throw runtime_error("Attempt to open/close/lock/unlock door on nonexistent exit! [" + room->id_str() + "]");
    if (!room->link_tag(dir, LinkTag::Openable)) throw runtime_error("Attempt to open/close/lock/unlock a non-Openable exit! [" + room->id_str() + "]");
    Room* dest_room = room->get_link(dir);
    const Direction reverse_dir = Room::reverse_direction(dir);
    std::string action_str;
    switch(type)
    {
        case OpenCloseLockUnlock::OPEN:
            room->set_link_tag(dir, LinkTag::Open);
            dest_room->set_link_tag(reverse_dir, LinkTag::Open);
            action_str = "opens";
            break;
        case OpenCloseLockUnlock::CLOSE:
            room->clear_link_tag(dir, LinkTag::Open);
            dest_room->clear_link_tag(reverse_dir, LinkTag::Open);
            action_str = "closes";
            break;
        case OpenCloseLockUnlock::LOCK:
            room->set_link_tag(dir, LinkTag::Locked);
            dest_room->set_link_tag(reverse_dir, LinkTag::Locked);
            action_str = "locks";
            break;
        case OpenCloseLockUnlock::UNLOCK:
            room->clear_link_tag(dir, LinkTag::Locked);
            dest_room->clear_link_tag(reverse_dir, LinkTag::Locked);
            action_str = "unlocks";
            break;
    }
    const Room* player_parent = player().parent_room();

    // Check if the player saw this door open or close.
    const bool player_sees = (room == player_parent || dest_room == player_parent) && (!actor || actor->type() != EntityType::PLAYER);
    if (!player_sees) return;
    const Direction player_sees_dir = (room == player_parent ? dir : reverse_dir);
    const string door_name = (room == player_parent ? room->door_name(dir) : dest_room->door_name(reverse_dir));

    // If the Actor pointer exists (it can be nullptr to make a door open on its own) AND the Mobile is in the same room as the Player, we'll inform them that
    // the door has been opened by this Mobile.
    std::string message;
    if (room == player_parent && actor && actor->parent_room() == player_parent)
    {
        if (actor->tag(EntityTag::PluralName)) action_str.pop_back();
        message = "{b}" + actor->name(NAME_FLAG_THE | NAME_FLAG_CAPITALIZE_FIRST) + " " + action_str + " the " + door_name;
        if (player_sees_dir == Direction::UP) message += " above.";
        else if (player_sees_dir == Direction::DOWN) message += " below.";
        else message += " to the " + Room::direction_name(player_sees_dir) + ".";
    }
    else
    {
        message = "{b}The " + door_name;
        if (player_sees_dir == Direction::UP) message += " above ";
        else if (player_sees_dir == Direction::DOWN) message += " below ";
        else message += " to the " + Room::direction_name(player_sees_dir) + " ";
        message += action_str + ".";
    }
    print(message);
}

// Saves the game! Should only be called via Game::save().
void World::save(int save_slot)
{
    for (auto &region : regions_)
        region.second->save_delta(save_slot);
}

// Returns a reference to the time/weather manager object.
TimeWeather& World::time_weather() const
{
    if (!time_weather_ptr_) throw runtime_error("Attempt to access null TimeWeather object!");
    return *time_weather_ptr_;
}

// Removes a Region from memory, saving it first.
void World::unload_region(int id)
{
    if (player().region() == id) throw runtime_error("Attempt to unload player-occupied region!");
    auto region = regions_.find(id);
    if (region == regions_.end()) return;   // It's not currently loaded.
    region->second->save_delta(game().save_slot());
    regions_.erase(region);
}

// Shortcut instead of using game()->world()
World& world() { return game().world(); }

}   // namespace westgate
