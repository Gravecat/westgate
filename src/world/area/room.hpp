// world/area/room.hpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <list>
#include <map>
#include <set>

#include "util/math/vector3.hpp"
#include "world/entity/entity.hpp"

namespace westgate {

class FileReader;   // defined in util/file/filereader.hpp
class FileWriter;   // defined in util/file/filewriter.hpp

// Cardinal directions, along with up/down, to link the world together.
enum class Direction : uint8_t { NONE, NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, UP, DOWN };

enum class RoomTag : uint16_t {
    // Tags regarding changes made to this Room.
    ChangedTags =       1,  // The RoomTags on this Room have been changed.
    ChangedDesc =       2,  // The Room's description has been changed.
    ChangedExits =      3,  // One or more exits in this Room have changed.
    ChangedShortName =  4,  // The short name of this Room has been changed.
    ChangedMapChar =    5,  // The map character for this Room has been changed.

    // Basic room attributes.
    Explored =      100,    // Has the player visited this Room before?
};

class Room {
public:
    static constexpr uint32_t   ROOM_SAVE_VERSION = 6;  // The expected version for saving/loading binary game data.

    static const std::string&   direction_name(Direction dir);  // Gets the string name of a Direction enum.

                Room(); // Creates a blank Room with default values and no ID.
                Room(const std::string& new_id);    // Creates a Room with a specified ID.
    void        add_entity(std::unique_ptr<Entity> entity); // Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
    void        clear_tag(RoomTag the_tag, bool mark_delta = true); // Clears a RoomTag from this Room.
    void        clear_tags(std::list<RoomTag> tags_list, bool mark_delta = true);   // Clears multiple RoomTags at the same time.
    const Vector3   coords() const; // Retrieves the coordinates of this Room.
    const std::string&  desc() const;   // Retrieves the description of this Room.
    Room*       get_link(Direction dir);    // Gets the Room linked in the specified direction, or nullptr if none is linked.
    uint32_t    id() const;     // Retrieves the hashed ID of this Room.
    const std::string&  id_str() const; // Retrieves the string ID of this Room.
    void        load_delta(FileReader* file);   // Loads only the changes to this Room from a save file. Should only be called by a parent Region.
    void        look() const;   // Look around you. Just look around you.
    const std::string   map_char() const;   // Retrieves the map character for this Room.
    uint32_t    region() const; // Returns the ID of the Region this Room belongs to.
    void        save_delta(FileWriter* file);   // Saves only the changes to this Room in a save file. Should only be called by a parent Region.
    void        set_coords(Vector3 new_coords); // Sets the coordinates of this room. Does not affect delta, as this should only ever be done when loading YAML.
    void        set_desc(const std::string& new_desc, bool mark_delta = true);  // Sets the description of this Room.
    void        set_exit(Direction dir, uint32_t new_exit, bool mark_delta = true); // Sets an exit link from this Room to another.
    void        set_map_char(const std::string& new_char, bool mark_delta = true);  // Sets the map character for this Room.
    void        set_short_name(const std::string& new_short_name, bool mark_delta = true);  // Sets the short name of this Room.
    void        set_tag(RoomTag the_tag, bool mark_delta = true);   // Sets a RoomTag on this Room.
    void        set_tags(std::list<RoomTag> tags_list, bool mark_delta = true); // Sets multiple RoomTags at the same time.
    const std::string&  short_name() const; // Retrieves the short name of this Room.
    bool        tag(RoomTag the_tag) const; // Checks if a RoomTag is set on this Room.
    void        transfer(Entity* entity_ptr, Room* room_ptr);   // Transfers a specified Entity from this Room to a target Room.

protected:
    std::vector<std::unique_ptr<Entity>>    entities_;  // Entities that exist within this Room.

private:
    static constexpr uint32_t   ROOM_DELTA_END =        0;  // The marker to indicate the end of delta changes on this Room.
    static constexpr uint32_t   ROOM_DELTA_ENTITIES =   1;  // The delta tag to indicate entity data is below.
    static constexpr uint32_t   ROOM_DELTA_TAGS =       2;  // Any and all RoomTags that have changed on this Room.
    static constexpr uint32_t   ROOM_DELTA_DESC =       3;  // The Room description, if it's changed.
    static constexpr uint32_t   ROOM_DELTA_EXITS =      4;  // The Room's exits, if any have changed.
    static constexpr uint32_t   ROOM_DELTA_SHORT_NAME = 5;  // The Room's short name, if it's changed.
    static constexpr uint32_t   ROOM_DELTA_MAP_CHAR =   6;  // The Room's map character, if it's changed.#

    static std::map<Direction, std::string> direction_names_;   // Static map that converts a Direction enum into string names.

    Vector3     coords_;        // The coordinates of this Room in the game world.
    std::string desc_;          // The text description of this Room, as shown to the player.
    uint32_t    exits_[10];     // Links in the eight cardinal directions, and up/down, to other Rooms.
    uint32_t    id_;            // The Room's unique hashed ID.
    std::string id_str_;        // The Room's unique text ID.
    std::string map_char_;      // The character representing this Room on the minimap.
    std::string short_name_;    // The short name of this Room.
    std::set<RoomTag> tags_;    // Any and all tags on this Room.
};

}   // namespace westgate
