// world/area/room.cpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "util/text/hash.hpp"
#include "world/area/room.hpp"
#include "world/entity/mobile.hpp"
#include "world/entity/player.hpp"

using std::make_unique;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;

namespace westgate {

// Creates a blank Room with default values and no ID.
Room::Room() : desc_("Missing room description."), id_(0), name_{"Undefined Room", "undefined"} { }

// Creates a Room with a specified ID.
Room::Room(const string& new_id) : Room()
{
    id_str_ = new_id;
    id_ = hash::murmur3(new_id);
}

// Creates a blank Room, then loads its data from the specified FileReader.
Room::Room(FileReader* file) : Room() { load(file); }

// Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
void Room::add_entity(unique_ptr<Entity> entity)
{
    entity->set_parent_room(this);
    entities_.push_back(std::move(entity));
}

// Retrieves the description of this Room.
const string& Room::desc() const { return desc_; }

// Retrieves the hashed ID of this Room.
uint32_t Room::id() const { return id_; }

// Loads a Room's data from the specified FileReader, overwriting existing data.
void Room::load(FileReader* file)
{
    // Check that the save file version matches.
    const uint32_t save_version = file->read_data<uint32_t>();
    if (save_version != ROOM_SAVE_VERSION) throw runtime_error("Invalid room version in saved data (" + to_string(save_version) + ", expected " +
        to_string(ROOM_SAVE_VERSION) + ")");

    // Read the new Room IDs.
    id_ = file->read_data<uint32_t>();
    id_str_ = file->read_string();

    // Read the Room's name and description.
    name_[0] = file->read_string();
    name_[1] = file->read_string();
    desc_ = file->read_string();

    // Load any Entities in this Room.
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
    terminal::print(desc_);
}

// Retrieves the name of this Room.
const string& Room::name(bool full_name) const { return name_[full_name ? 0 : 1]; }

// Saves this Room to a specified save file. Should only be called by a parent Region.
void Room::save(FileWriter* file)
{
    // Write the save version for this Room.
    file->write_data<uint32_t>(ROOM_SAVE_VERSION);

    // Write the Room's IDs, both string and hashed versions.
    file->write_data<uint32_t>(id_);
    file->write_string(id_str_);

    // Write the Room's name and description.
    file->write_string(name_[0]);
    file->write_string(name_[1]);
    file->write_string(desc_);

    // Save any Entities in this Room.
    file->write_data<size_t>(entities_.size());
    for (auto &entity : entities_)
        entity->save(file);
}

// Sets the description of this Room.
void Room::set_desc(const string& new_desc)
{
    if (!new_desc.size())
    {
        core().nonfatal("Attempt to set blank description on room (" + name_[0] + ")", Core::CORE_ERROR);
        desc_ = "Missing room description.";
    }
    else desc_ = new_desc;
}

// Sets the name of this Room.
void Room::set_name(const string& new_full_name, const string& new_short_name)
{
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
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + name_[0] + " to itself.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer enity (" + entity_ptr->name() + ") from " + name_[0] + " to itself.", Core::CORE_ERROR);
        return;
    }
    if (!entity_ptr)
    {
        if (!room_ptr) core().nonfatal("Attempt to transfer null entity from " + name_[0] + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer null entity from " + name_[0] + " to " + room_ptr->name(true) + ".", Core::CORE_ERROR);
        return;
    }
    if (!room_ptr)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + name_[0] + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + name_[0] + " to null room.", Core::CORE_ERROR);
        return;
    }
    if (entity_ptr->parent_room() != this)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + name_[0] + " to " + room_ptr->name(true) +
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
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + name_[0] + " to " + room_ptr->name(true) +
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
