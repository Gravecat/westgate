// world/entity/entity.cpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "world/entity/entity.hpp"
#include "world/room.hpp"

namespace lom {

// Creates a blank Entity with default values.
Entity::Entity() : gender_(Gender::NONE), name_("undefined entity"), parent_entity_(nullptr), parent_room_(nullptr) { }

// Retrieves the gender (if any) of this Entity.
Gender Entity::gender() const { return gender_; }

// Retrieves the name of this Entity.
const std::string& Entity::name() const { return name_; }

// Retrieves the Entity (if any) containing this Entity.
const Entity* Entity::parent_entity() const { return parent_entity_; }

// Retrieves the Room (if any) containing this Entity.
const Room* Entity::parent_room() const { return parent_room_; }

// Sets the gender of this Entity.
void Entity::set_gender(Gender new_gender)
{
    // While unlikely, it can't hurt to check and ensure the value is within valid bounds.
    if (static_cast<uint8_t>(new_gender) > static_cast<uint8_t>(Gender::IT))
    {
        core().nonfatal("Attempt to set invalid gender (" + std::to_string(static_cast<uint8_t>(new_gender)) + ") on " + name_, Core::CORE_ERROR);
        new_gender = Gender::NONE;
    }
    gender_ = new_gender;
}

// Sets the name of this Entity.
void Entity::set_name(const std::string& new_name) { name_ = new_name; }

// Sets a new Entity as the parent of this Entity, or nullptr for none.
void Entity::set_parent_entity(Entity* new_entity_parent)
{
    if (new_entity_parent == this)
    {
        core().nonfatal("Attempt to set entity parent to itself (" + name_ + ")", Core::CORE_ERROR);
        new_entity_parent = nullptr;
    }
    parent_entity_ = new_entity_parent;
    if (new_entity_parent) set_parent_room(nullptr);    // An Entity can only have one parent.
}

// Sets a new Room as the parent of this Entity, or nullptr for none.
void Entity::set_parent_room(Room* new_room_parent)
{
    parent_room_ = new_room_parent;
    if (new_room_parent) set_parent_entity(nullptr);    // An Entity can only have one parent.
}

// Retrieves the world coordinates of the parent of this Entity, or {0,0} if no parent is defined.
const Vector2 Entity::world_pos() const
{
    if (parent_room_) return parent_room_->world_pos();
    else if (parent_entity_)
    {
        // Sanity check to avoid an infinite loop. This will only really be possible if something was badly broken in the code, but it'll help with debugging.
        // It should be impossible for this to happen, but hey, it's such a simple quick check that'll avoid some nasty infinite recursion.
        if (parent_entity_ == this)
        {
            core().nonfatal("Attempt to check world position of Entity parented to itself (" + name_ + ")", Core::CORE_ERROR);
            return {0,0};
        }
        return parent_entity_->world_pos();
    }
    else return {0,0};
}

}   // namespace lom
