// world/area/room.hpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <map>

#include "world/entity/entity.hpp"

namespace westgate {

class FileReader;   // defined in util/file/filereader.hpp
class FileWriter;   // defined in util/file/filewriter.hpp

// Cardinal directions, along with up/down, to link the world together.
enum class Direction : uint8_t { NONE, NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, UP, DOWN };

class Room {
public:
    static constexpr uint32_t   ROOM_SAVE_VERSION = 4;  // The expected version for saving/loading binary game data.

                Room(); // Creates a blank Room with default values and no ID.
                Room(const std::string& new_id);    // Creates a Room with a specified ID.
    void        add_entity(std::unique_ptr<Entity> entity); // Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
    const std::string&  desc() const;   // Retrieves the description of this Room.
    const std::string&  direction_name(Direction dir) const;    // Gets the string name of a Direction enum.
    Room*       get_link(Direction dir);    // Gets the Room linked in the specified direction, or nullptr if none is linked.
    uint32_t    id() const;     // Retrieves the hashed ID of this Room.
    const std::string&  id_str() const; // Retrieves the string ID of this Room.
    void        load_delta(FileReader* file);   // Loads only the changes to this Room from a save file. Should only be called by a parent Region.
    void        look() const;   // Look around you. Just look around you.
    const std::string&  name(bool full_name) const; // Retrieves the name of this Room.
    uint32_t    region() const; // Returns the ID of the Region this Room belongs to.
    void        save_delta(FileWriter* file);   // Saves only the changes to this Room in a save file. Should only be called by a parent Region.
    void        set_desc(const std::string& new_desc, bool mark_delta = true);  // Sets the description of this Room.
    void        set_exit(Direction dir, uint32_t new_exit, bool mark_delta = true); // Sets an exit link from this Room to another.
    void        set_name(const std::string& new_full_name, const std::string& new_short_name, bool mark_delta = true);  // Sets the name of this Room.
    void        transfer(Entity* entity_ptr, Room* room_ptr);   // Transfers a specified Entity from this Room to a target Room.

protected:
    std::vector<std::unique_ptr<Entity>>    entities_;  // Entities that exist within this Room.

private:
    static constexpr uint32_t   ROOM_DELTA_ENTITIES =   1;  // The delta tag to indicate entity data is below.

    static std::map<Direction, std::string> direction_names_;   // Static map that converts a Direction enum into string names.

    std::string desc_;      // The text description of this Room, as shown to the player.
    uint32_t    exits_[10]; // Links in the eight cardinal directions, and up/down, to other Rooms.
    uint32_t    id_;        // The Room's unique hashed ID.
    std::string id_str_;    // The Room's unique text ID.
    std::string name_[2];   // The name of this Room, both in full-name and short-name form.
};

}   // namespace westgate
