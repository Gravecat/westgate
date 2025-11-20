// world/room.cpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "world/entity/mobile.hpp"
#include "world/room.hpp"

namespace westgate {

// Creates a blank Room with default values.
Room::Room() : desc_("Missing room description."), name_("Undefined Room"), world_pos_({0,0}) { }

// Retrieves the description of this Room.
const std::string& Room::desc() const { return desc_; }

// Retrieves the name of this Room.
const std::string& Room::name() const { return name_; }

// Sets the description of this Room.
void Room::set_desc(const std::string& new_desc)
{
    if (!new_desc.size())
    {
        core().nonfatal("Attempt to set blank description on room (" + name_ + ")", Core::CORE_ERROR);
        desc_ = "Missing room description.";
    }
    else desc_ = new_desc;
}

// Sets the name of this Room.
void Room::set_name(const std::string& new_name)
{
    if (!new_name.size())
    {
        core().nonfatal("Attempt to set blank name on room.", Core::CORE_ERROR);
        name_ = "Undefined Room";
    }
    else name_ = new_name;
}

// Sets the new world coordinates for this Room.
void Room::set_world_pos(const Vector2 new_pos)
{
    if (!new_pos)
    {
        core().nonfatal("Attempt to set {0,0} coordinates on room (" + name_ + ")", Core::CORE_ERROR);
        return;
    }
    world_pos_ = new_pos;
}

// Transfers a specified Mobile from this Room to a target Room.
void Room::transfer(Entity* entity_ptr, Room* room_ptr)
{
    // First, sanity checks. These should never happen, but could possibly occur as the result of mistakes in the code.
    if (room_ptr == this)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + name_ + " to itself.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer enity (" + entity_ptr->name() + ") from " + name_ + " to itself.", Core::CORE_ERROR);
        return;
    }
    if (!entity_ptr)
    {
        if (!room_ptr) core().nonfatal("Attempt to transfer null entity from " + name_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer null entity from " + name_ + " to " + room_ptr->name() + ".", Core::CORE_ERROR);
        return;
    }
    if (!room_ptr)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + name_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + name_ + " to null room.", Core::CORE_ERROR);
        return;
    }
    if (entity_ptr->parent_room() != this)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + name_ + " to " + room_ptr->name() +
            " while entity is not correctly parented to this room.", Core::CORE_ERROR);
        return;
    }

    // Try to determine which vector position houses the Entity being moved.
    uint32_t source_id = 0;
    bool source_found = false;
    for (unsigned int i = 0; i < entities_.size(); i++)
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
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + name_ + " to " + room_ptr->name() +
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

// Retrieves the world coordinates of this Room.
const Vector2 Room::world_pos() const
{
    if (!world_pos_) core().nonfatal("Coordinate lookup for room at {0,0} (" + name_ + ")", Core::CORE_WARN);
    return world_pos_;
}

}   // namespace westgate
