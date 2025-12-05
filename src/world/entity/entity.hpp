// world/entity/entity.hpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

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

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <list>
#include <set>

namespace westgate {

class FileReader;   // defined in util/filex.hpp
class FileWriter;   // defined in util/filex.hpp
class Room;         // defined in world/room.hpp

// Used to allow Entity to self-identify its derived class.
enum class EntityType : uint8_t { ENTITY, MOBILE, PLAYER };

// The genders of things in the game.
enum class Gender : uint8_t { NONE, HE, SHE, THEY, IT };

enum class EntityTag : uint16_t {
    // Tags regarding this Entity's name or identity.
    ProperNoun = 1,     // This Entity's name is a proper noun (e.g. Smaug).
    PluralName,         // This Entity's name is a plural (e.g. "pack of rats").
    Construct,          // This Entity is a construct, or something else not alive.
};

// Flags for the name() function.
static constexpr uint32_t NAME_FLAG_THE =               1;  // Precede the Entity's name with 'the', unless the name is a proper noun.
static constexpr uint32_t NAME_FLAG_CAPITALIZE_FIRST =  2;  // Capitalize the first letter of the Entity's name (including the "The") if set.
static constexpr uint32_t NAME_FLAG_POSSESSIVE =        4;  // Change the Entity's name to a possessive noun (e.g. goblin -> goblin's).
static constexpr uint32_t NAME_FLAG_PLURAL =            8;  // Return a plural of the Entity's name (e.g. apple -> apples).

class Entity {
public:
                        Entity() = delete;  // No default constructor; use nullptr on the constructor below.
                        Entity(FileReader* file);   // Creates a blank Entity, then loads its data from a FileReader.
    virtual             ~Entity() = default;    // Virtual destructor. Nothing here yet.
    void                clear_tag(EntityTag the_tag);   // Clears an EntityTag from this Entity.
    void                clear_tags(std::list<EntityTag> tags_list); // Clears multiple EntityTags at the same time.
    Gender              gender() const; // Retrieves the gender (if any) of this Entity.
    const std::string   he_she(bool capitalize_first = false) const;    // Returns a gender string (he/she/it/they/etc.)
    const std::string   himself_herself() const;        // Returns a gender string (himself/herself/theirself/etc.)
    const std::string   his_her() const;                // Returns a gender string (his/her/its/their/etc.)
    const std::string   name(uint32_t flags = 0) const; // Retrieves the name of this Entity.
    Entity*             parent_entity() const;  // Retrieves the Entity (if any) containing this Entity.
    Room*               parent_room() const;    // Retrieves the Room (if any) containing this Entity.
    virtual void        save(FileWriter* file); // Saves this Entity to a save game file.
    void                set_gender(Gender new_gender);  // Sets the gender of this Entity.
    void                set_name(const std::string& new_name);  // Sets the name of this Entity.
    virtual void        set_parent_entity(Entity* new_entity_parent = nullptr); // Sets a new Entity as the parent of this Entity, or nullptr for none.
    virtual void        set_parent_room(Room* new_room_parent = nullptr);       // Sets a new Room as the parent of this Entity, or nullptr for none.
    void                set_tag(EntityTag the_tag);     // Sets an EntityTag on this Entity.
    void                set_tags(std::list<EntityTag> tags_list);   // Sets multiple EntityTags at the same time.
    bool                tag(EntityTag the_tag) const;   // Checks if an EntityTag is set on this Entity.
    void                toggle_tag(EntityTag the_tag);  // Toggles an EntityTag on or off.
    virtual EntityType  type() const { return EntityType::ENTITY; } // Self-identifies this Entity's derived class.

protected:
    Gender      gender_;        // The gender of this Entity, if any.
    std::string name_;          // Every Entity must be called something.
    Entity*     parent_entity_; // The Entity (if any) containing this Entity.
    Room*       parent_room_;   // The Room (if any) where this Entity is located.

private:
    static constexpr uint32_t   ENTITY_SAVE_VERSION =   3;  // The expected version for saving/loading binary game data.

    // Identifiers for blocks of data in the save file, used to quickly catch errors when loading old or invalid data.
    static constexpr uint32_t   ENTITY_SAVE_PROPS =     1;
    static constexpr uint32_t   ENTITY_SAVE_TAGS =      2;

    std::set<EntityTag> tags_;  // Any and all tags on this Entity.
};

}   // namespace westgate
