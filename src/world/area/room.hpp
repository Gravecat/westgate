// world/area/room.hpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

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
#include <map>
#include <set>

#include "world/area/link.hpp"
#include "world/entity/entity.hpp"

namespace westgate {

class FileReader;   // defined in util/filex.hpp
class FileWriter;   // defined in util/filex.hpp

// Tags are kinda like flags that can be set on Rooms, except using std::set.
enum class RoomTag : uint16_t {
    // Tags regarding changes made to this Room.
    ChangedTags =       1,  // The RoomTags on this Room have been changed.
    ChangedDesc =       2,  // The Room's description has been changed.
    ChangedExits =      3,  // The Room's exits have been changed.
    ChangedName =       4,  // The name of this Room has been changed.
    ChangedMapChar =    5,  // The map character for this Room has been changed.

    // Basic room attributes.
    Explored =      100,    // Has the player visited this Room before?

    // Room attributes regarding the time/weather system.
    Indoors =       201,    // Is this Room indoors?
    Windows =       202,    // Is this Room an indoors location with windows?
    City =          203,    // Is this Room part of a city?
    Underground =   204,    // Is this Room underground?
    Trees =         205,    // Are there trees nearby?
    AlwaysWinter =  206,    // The weather system will be locked to winter for this room.
    AlwaysSpring =  207,    // The weather system will be locked to spring for this room.
    AlwaysSummer =  208,    // The weather system will be locked to summer for this room.
    AlwaysAutumn =  209,    // The weather system will be locked to autumn (fall) for this room.

    // Markers for exits in a Room that are planned but currently unfinished.
    UnfinishedNorth =       300,    // Marked in-game as unfinished, show on the map as red links.
    UnfinishedNortheast =   301,
    UnfinishedEast =        302,
    UnfinishedSoutheast =   303,
    UnfinishedSouth =       304,
    UnfinishedSouthwest =   305,
    UnfinishedWest =        306,
    UnfinishedNorthwest =   307,
    UnfinishedUp =          308,
    UnfinishedDown =        309,
    PermalockNorth =        310,    // Fake room exits, which act like locked doors that can never be opened.
    PermalockNortheast =    311,
    PermalockEast =         312,
    PermalockSoutheast =    313,
    PermalockSouth =        314,
    PermalockSouthwest =    315,
    PermalockWest =         316,
    PermalockNorthwest =    317,
    PermalockUp =           318,
    PermalockDown =         319,
};

class Room {
public:
    static constexpr uint32_t   ROOM_SAVE_VERSION = 9;  // The expected version for saving/loading binary game data.

    static const std::string&   direction_name(Direction dir);  // Gets the string name of a Direction enum.
    static RoomTag              parse_room_tag(const std::string &tag); // Parses a string RoomTag name into a RoomTag enum.
    static Direction            reverse_direction(Direction dir);   // Reverses a Direction (e.g. north becomes south).

                Room(); // Creates a blank Room with default values and no ID.
                Room(const std::string& new_id);    // Creates a Room with a specified ID.
    void        add_entity(std::unique_ptr<Entity> entity); // Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
    bool        can_see_outside() const;    // Checks if we can see the outside world from here.
    void        clear_link_tag(Direction dir, LinkTag the_tag, bool mark_delta = true); // Clears a LinkTag from a specified Link.
    void        clear_link_tags(Direction dir, std::list<LinkTag> tags_list, bool mark_delta = true);   // Clears multiple LinkTags at once.
    void        clear_tag(RoomTag the_tag, bool mark_delta = true); // Clears a RoomTag from this Room.
    void        clear_tags(std::list<RoomTag> tags_list, bool mark_delta = true);   // Clears multiple RoomTags at the same time.
    const std::string   door_name(Direction dir) const; // Returns the name of the door (door, gate, etc.) on the specified Link, if any.
    Room*       get_link(Direction dir);    // Gets the Room linked in the specified direction, or nullptr if none is linked.
    bool        has_exit(Direction dir) const;  // Checks if an Exit exists in the specified Direction.
    uint32_t    id() const;     // Retrieves the hashed ID of this Room.
    const std::string&  id_str() const; // Retrieves the string ID of this Room.
    bool        is_unfinished(Direction dir, bool permalock) const; // Checks if this Room has an unfinished or permalock link in a specified direction.
    bool        link_tag(Direction dir, LinkTag tag) const; // Checks a LinkTag on a specified Link.
    void        load_delta(FileReader* file);   // Loads only the changes to this Room from a save file. Should only be called by a parent Region.
    void        look(); // Look around you. Just look around you.
    const std::string   map_char() const;   // Retrieves the map character for this Room.
    const std::string&  name() const;   // Retrieves the full name of this Room.
    int         region() const; // Returns the ID of the Region this Room belongs to.
    void        save_delta(FileWriter* file);   // Saves only the changes to this Room in a save file. Should only be called by a parent Region.
    void        set_desc(const std::string& new_desc, bool mark_delta = true);  // Sets the description of this Room.
    void        set_link(Direction dir, uint32_t new_exit, bool mark_delta = true); // Sets an exit link from this Room to another.
    void        set_link_tag(Direction dir, LinkTag tag, bool mark_delta = true);   // Sets a LinkTag on a specifieid Link.
    void        set_link_tags(Direction dir, std::list<LinkTag> tags_list, bool mark_delta = true); // Sets multiple LinkTags at once.
    void        set_map_char(const std::string& new_char, bool mark_delta = true);  // Sets the map character for this Room.
    void        set_name(const std::string& new_name = "", const std::string& new_short_name = "", bool mark_delta = true); // Sets the name of this Room.
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
    static constexpr uint32_t   ROOM_DELTA_LINKS =      4;  // The Room's Links, if any have changed.
    static constexpr uint32_t   ROOM_DELTA_NAME =       5;  // The Room's name, if it's changed.
    static constexpr uint32_t   ROOM_DELTA_MAP_CHAR =   6;  // The Room's map character, if it's changed.

    static constexpr uint32_t   ROOM_DELTA_LINK_NONE =      100;    // Marks this Link as missing or removed.
    static constexpr uint32_t   ROOM_DELTA_LINK_UNCHANGED = 101;    // Marks this Link as existing but unchanged.
    static constexpr uint32_t   ROOM_DELTA_LINK_CHANGED =   201;    // Marks this Link as existing and modified.

    static const std::string    direction_names_[11];       // Lookup table to convert a Direction enum into a string name.
    static const Direction      reverse_direction_map_[11]; // Lookup table that inverts a Direction (e.g. east -> west).
    static const std::map<std::string, RoomTag> tag_map_;   // Used during loading YAML data, to convert RoomTag text names into RoomTag enums.
    static const RoomTag        unfinished_directions_[20]; // Lookup table for unfinished exit links.

    // Turns a Direction into an int for array access, produces a standard error on invalid input.
    int link_id(Direction dir, const std::string& caller, bool fail_on_null = true) const;

    std::string desc_;          // The text description of this Room, as shown to the player.
    std::unique_ptr<Link>   links_[10]; // Any and all Links leading out of this Room.
    uint32_t    id_;            // The Room's unique hashed ID.
    std::string id_str_;        // The Room's unique text ID.
    std::string map_char_;      // The character representing this Room on the minimap.
    std::string name_[2];       // The long and short name of this Room.
    std::set<RoomTag> tags_;    // Any and all tags on this Room.
};

}   // namespace westgate
