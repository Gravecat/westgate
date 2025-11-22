// world/entity/entity.cpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "world/area/room.hpp"
#include "world/entity/entity.hpp"

using std::runtime_error;
using std::string;
using std::to_string;

namespace westgate {

// Creates a blank Entity, then loads its data from a FileReader.
Entity::Entity(FileReader* file) : gender_(Gender::NONE), name_("undefined entity"), parent_entity_(nullptr), parent_room_(nullptr)
{
    if (!file) return;

    // Check the save version for this Entity.
    const uint32_t save_version = file->read_data<uint32_t>();
    if (save_version != ENTITY_SAVE_VERSION)
        throw runtime_error("Invalid entity save version (" + to_string(save_version) + " (expected " + to_string(ENTITY_SAVE_VERSION) + ")");

    // Retrieve the Entity's name and gender.
    name_ = file->read_string();
    gender_ = file->read_data<Gender>();
}

// Retrieves the gender (if any) of this Entity.
Gender Entity::gender() const { return gender_; }

// Retrieves the name of this Entity.
const string& Entity::name() const { return name_; }

// Retrieves the Entity (if any) containing this Entity.
const Entity* Entity::parent_entity() const { return parent_entity_; }

// Retrieves the Room (if any) containing this Entity.
const Room* Entity::parent_room() const { return parent_room_; }

// Saves this Entity to a save game file.
void Entity::save(FileWriter* file)
{
    // Write this Entity's type identifier. This will be critical when loading Entities later.
    file->write_data<EntityType>(type());

    // Write the save version for this Entity.
    file->write_data<uint32_t>(ENTITY_SAVE_VERSION);

    // Write the Entity's name and gender.
    file->write_string(name_);
    file->write_data<Gender>(gender_);
}

// Sets the gender of this Entity.
void Entity::set_gender(Gender new_gender)
{
    // While unlikely, it can't hurt to check and ensure the value is within valid bounds.
    if (static_cast<uint8_t>(new_gender) > static_cast<uint8_t>(Gender::IT))
    {
        core().nonfatal("Attempt to set invalid gender (" + to_string(static_cast<uint8_t>(new_gender)) + ") on " + name_, Core::CORE_ERROR);
        new_gender = Gender::NONE;
    }
    gender_ = new_gender;
}

// Sets the name of this Entity.
void Entity::set_name(const string& new_name) { name_ = new_name; }

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

}   // namespace westgate
