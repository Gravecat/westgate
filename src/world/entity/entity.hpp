// world/entity/entity.hpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

namespace westgate {

class Room; // defined in world/room.hpp

// The genders of things in the game.
enum class Gender : uint8_t { NONE, HE, SHE, THEY, IT };

class Entity {
public:
                        Entity();   // Creates a blank Entity with default values.
    virtual             ~Entity() = default;    // Virtual destructor. Nothing here yet.
    Gender              gender() const; // Retrieves the gender (if any) of this Entity.
    const std::string&  name() const;   // Retrieves the name of this Entity.
    const Entity*       parent_entity() const;  // Retrieves the Entity (if any) containing this Entity.
    const Room*         parent_room() const;    // Retrieves the Room (if any) containing this Entity.
    void                set_gender(Gender new_gender);  // Sets the gender of this Entity.
    void                set_name(const std::string& new_name);  // Sets the name of this Entity.
    void                set_parent_entity(Entity* new_entity_parent = nullptr); // Sets a new Entity as the parent of this Entity, or nullptr for none.
    void                set_parent_room(Room* new_room_parent = nullptr);       // Sets a new Room as the parent of this Entity, or nullptr for none.

protected:
    Gender      gender_;        // The gender of this Entity, if any.
    std::string name_;          // Every Entity must be called something.
    Entity*     parent_entity_; // The Entity (if any) containing this Entity.
    Room*       parent_room_;   // The Room (if any) where this Entity is located.
};

}   // namespace westgate
