// world/entity/entity.hpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "core/global.hpp"

namespace lom {

// The genders of things in the game.
enum class Gender : uint8_t { NONE, HE, SHE, THEY, IT };

class Entity {
public:
                        Entity();   // Creates a blank Entity with default values.
    virtual             ~Entity() = default;    // Virtual destructor. Nothing here yet.
    Gender              gender() const; // Retrieves the gender (if any) of this Entity.
    const std::string&  name() const;   // Retrieves the name of this Entity.
    void                set_gender(Gender new_gender);  // Sets the gender of this Entity.
    void                set_name(const std::string& new_name);  // Sets the name of this Entity.

protected:
    Gender      gender_;    // The gender of this Entity, if any.
    std::string name_;      // Every Entity must be called something.
};

}   // namespace lom
