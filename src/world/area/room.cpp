// world/area/room.cpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <vector>

#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "util/text/hash.hpp"
#include "util/text/stringutils.hpp"
#include "world/area/region.hpp"
#include "world/area/room.hpp"
#include "world/entity/mobile.hpp"
#include "world/entity/player.hpp"
#include "world/world.hpp"

using std::make_unique;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

namespace westgate {

// Static map that converts a Direction enum into string names.
std::map<Direction, std::string> Room::direction_names_ = {
    { Direction::NORTH, "north" }, { Direction::NORTHEAST, "northeast" }, { Direction::EAST, "east" }, { Direction::SOUTHEAST, "southeast" },
    { Direction::SOUTH, "south" }, { Direction::SOUTHWEST, "southwest"}, { Direction::WEST, "west" }, { Direction::NORTHWEST, "northwest "},
    { Direction::UP, "up" }, { Direction::DOWN, "down" }, { Direction::NONE, "" }
};

// Creates a blank Room with default values and no ID.
Room::Room() : desc_("Missing room description."), exits_{}, id_(0), name_{"Undefined Room", "undefined"} { }

// Creates a Room with a specified ID.
Room::Room(const string& new_id) : Room()
{
    id_str_ = new_id;
    id_ = hash::murmur3(new_id);
}

// Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
void Room::add_entity(unique_ptr<Entity> entity)
{
    entity->set_parent_room(this);
    entities_.push_back(std::move(entity));
}

// Retrieves the description of this Room.
const string& Room::desc() const { return desc_; }

// Gets the string name of a Direction enum.
const std::string& Room::direction_name(Direction dir)
{
    auto result = direction_names_.find(dir);
    if (result == direction_names_.end())
    {
        core().nonfatal("Unable to parse direction enum.", Core::CORE_ERROR);
        return direction_names_.find(Direction::NONE)->second;
    }
    return result->second;
}

// Gets the Room linked in the specified direction, or nullptr if none is linked.
Room* Room::get_link(Direction dir)
{
    const uint8_t array_pos = static_cast<uint8_t>(dir) - 1;
    if (array_pos >= 10)
    {
        core().nonfatal("Attempt to retrieve invalid room link on " + id_str_ + " (" + std::to_string(array_pos) + ")", Core::CORE_ERROR);
        return nullptr;
    }
    if (!exits_[array_pos]) return nullptr;
    return world().find_room(exits_[array_pos]);
}

// Retrieves the hashed ID of this Room.
uint32_t Room::id() const { return id_; }

// Retrieves the string ID of this Room.
const std::string& Room::id_str() const { return id_str_; }

// Loads only the changes to this Room from a save file. Should only be called by a parent Region.
void Room::load_delta(FileReader* file)
{
    // Load any Entities in this Room.
    const uint32_t entity_tag = file->read_data<uint32_t>();
    if (entity_tag != ROOM_DELTA_ENTITIES) throw runtime_error("Invalid delta tag in room data (" + to_string(entity_tag) + ", expected " +
        to_string(ROOM_DELTA_ENTITIES) + ")");

    const size_t entity_count = file->read_data<size_t>();
    entities_.reserve(entity_count);
    for (size_t i = 0; i < entity_count; i++)
    {
        EntityType type = file->read_data<EntityType>();
        switch(type)
        {
            case EntityType::ENTITY: add_entity(make_unique<Entity>(file)); break;
            case EntityType::MOBILE: add_entity(make_unique<Mobile>(file)); break;
            case EntityType::PLAYER: add_entity(make_unique<Player>(file)); break;
            default: throw runtime_error("Attempt to load unknown entity type: " + to_string(static_cast<int>(type)));
        }
    }
}

// Look around you. Just look around you.
void Room::look() const
{
    terminal::print("{C}" + name_[0]);
    terminal::print("  " + desc_);

    vector<string> exits_list;
    for (int i = 0; i < 10; i++)
    {
        const uint32_t exit = exits_[i];
        if (!exit) continue;
        exits_list.push_back(direction_name(static_cast<Direction>(i + 1)));
    }
    if (exits_list.size()) terminal::print("\n{C}[Exits: " + stringutils::comma_list(exits_list, stringutils::CL_MODE_USE_AND) + "]");
}

// Retrieves the name of this Room.
const string& Room::name(bool full_name) const { return name_[full_name ? 0 : 1]; }

// Returns the ID of the Region this Room belongs to.
uint32_t Room::region() const
{ return world().find_room_region(id_); }

// Saves only the changes to this Room in a save file. Should only be called by a parent Region.
void Room::save_delta(FileWriter* file)
{
    // Right now, we're not saving any Room data *except* for Entities.
    bool changes = (entities_.size() > 0);
    if (!changes) return;

    // Write the save version for this Room, and the Room's ID.
    file->write_data<uint32_t>(Region::REGION_DELTA_ROOM);
    file->write_data<uint32_t>(ROOM_SAVE_VERSION);
    file->write_data<uint32_t>(id_);

    // Save any Entities in this Room.
    file->write_data<uint32_t>(ROOM_DELTA_ENTITIES);
    file->write_data<size_t>(entities_.size());
    for (auto &entity : entities_)
        entity->save(file);
}

// Sets the description of this Room.
void Room::set_desc(const string& new_desc, bool mark_delta)
{
    (void)mark_delta;   // We'll use this later.

    if (!new_desc.size())
    {
        core().nonfatal("Attempt to set blank description on room (" + id_str_ + ")", Core::CORE_ERROR);
        desc_ = "Missing room description.";
    }
    else desc_ = new_desc;
}

// Sets an exit link from this Room to another.
void Room::set_exit(Direction dir, uint32_t new_exit, bool mark_delta)
{
    (void)mark_delta;   // We'll use this later.

    if (dir == Direction::NONE || dir > Direction::DOWN) throw std::runtime_error("Invalid direction on set_exit call (" + id_str_ + ")");
    exits_[static_cast<uint8_t>(dir) - 1] = new_exit;
}

// Sets the name of this Room.
void Room::set_name(const string& new_full_name, const string& new_short_name, bool mark_delta)
{
    (void)mark_delta;   // We'll use this later.

    if (!new_full_name.size() || !new_short_name.size())
    {
        core().nonfatal("Attempt to set blank name on room.", Core::CORE_ERROR);
        name_[0] = "Undefined Room";
        name_[1] = "undefined";
    }
    else
    {
        name_[0] = new_full_name;
        name_[1] = new_short_name;
    }
}

// Transfers a specified Entity from this Room to a target Room.
void Room::transfer(Entity* entity_ptr, Room* room_ptr)
{
    // First, sanity checks. These should never happen, but could possibly occur as the result of mistakes in the code.
    if (room_ptr == this)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to itself.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer enity (" + entity_ptr->name() + ") from " + id_str_ + " to itself.", Core::CORE_ERROR);
        return;
    }
    if (!entity_ptr)
    {
        if (!room_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to " + room_ptr->id_str() + ".", Core::CORE_ERROR);
        return;
    }
    if (!room_ptr)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to null room.", Core::CORE_ERROR);
        return;
    }
    if (entity_ptr->parent_room() != this)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to " + room_ptr->id_str() +
            " while entity is not correctly parented to this room.", Core::CORE_ERROR);
        return;
    }

    // Try to determine which vector position houses the Entity being moved.
    uint32_t source_id = 0;
    bool source_found = false;
    for (size_t i = 0; i < entities_.size(); i++)
    {
        if (entities_.at(i).get() == entity_ptr)
        {
            source_id = i;
            source_found = true;
            break;
        }
    }
    if (!source_found)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to " + room_ptr->id_str() +
            ", while entity is not contained within the parent room.", Core::CORE_ERROR);
        return;
    }

    // We now know the position in the entities_ vector for the requested entity, and all the pointers appear to be in order. Let's transfer ownership...
    room_ptr->entities_.push_back(std::move(entities_[source_id]));
    // ...and immediately delete the now-invalid pointer from our own entities_ vector...
    entities_.erase(entities_.begin() + source_id);
    // ...and finally, update the Entity's parent Room pointer.
    entity_ptr->set_parent_room(room_ptr);
}

}   // namespace westgate
