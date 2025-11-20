// world/entity/entity.cpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "world/entity/entity.hpp"

namespace lom {

// Creates a blank Entity with default values.
Entity::Entity() : gender_(Gender::NONE), name_("undefined entity") { }

// Retrieves the gender (if any) of this Entity.
Gender Entity::gender() const { return gender_; }

// Retrieves the name of this Entity.
const std::string& Entity::name() const { return name_; }

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

}   // namespace lom
