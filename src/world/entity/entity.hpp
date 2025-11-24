// world/entity/entity.hpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {

class FileReader;   // defined in util/file/filereader.hpp
class FileWriter;   // defined in util/file/filewriter.hpp
class Room;         // defined in world/room.hpp

// Used to allow Entity to self-identify its derived class.
enum class EntityType : uint8_t { ENTITY, MOBILE, PLAYER };

// The genders of things in the game.
enum class Gender : uint8_t { NONE, HE, SHE, THEY, IT };

class Entity {
public:
                        Entity() = delete;  // No default constructor; use nullptr on the constructor below.
                        Entity(FileReader* file);   // Creates a blank Entity, then loads its data from a FileReader.
    virtual             ~Entity() = default;    // Virtual destructor. Nothing here yet.
    Gender              gender() const; // Retrieves the gender (if any) of this Entity.
    const std::string&  name() const;   // Retrieves the name of this Entity.
    Entity*             parent_entity() const;  // Retrieves the Entity (if any) containing this Entity.
    Room*               parent_room() const;    // Retrieves the Room (if any) containing this Entity.
    virtual void        save(FileWriter* file); // Saves this Entity to a save game file.
    void                set_gender(Gender new_gender);  // Sets the gender of this Entity.
    void                set_name(const std::string& new_name);  // Sets the name of this Entity.
    virtual void        set_parent_entity(Entity* new_entity_parent = nullptr); // Sets a new Entity as the parent of this Entity, or nullptr for none.
    virtual void        set_parent_room(Room* new_room_parent = nullptr);       // Sets a new Room as the parent of this Entity, or nullptr for none.
    virtual EntityType  type() const { return EntityType::ENTITY; } // Self-identifies this Entity's derived class.

protected:
    Gender      gender_;        // The gender of this Entity, if any.
    std::string name_;          // Every Entity must be called something.
    Entity*     parent_entity_; // The Entity (if any) containing this Entity.
    Room*       parent_room_;   // The Room (if any) where this Entity is located.

private:
    static constexpr uint32_t   ENTITY_SAVE_VERSION =   2;  // The expected version for saving/loading binary game data.
};

}   // namespace westgate
