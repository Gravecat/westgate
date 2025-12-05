// world/area/link.hpp -- A Link is a connection between two Rooms. It can have its own special properties, such as being a door.

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
#include "core/pch.hpp"

#include <list>
#include <map>
#include <set>

namespace westgate {

class FileReader;   // defined in util/filex.hpp
class FileWriter;   // defined in util/filex.hpp

// Cardinal directions, along with up/down, to link the world together.
enum class Direction : uint8_t { NONE, NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, UP, DOWN };

// Tags are kinda like flags that can be set on Links, except using std::set.
enum class LinkTag : uint16_t {
    // Tags regarding changes made to this exit.
    ChangedLink =   1,  // The exit link has changed.
    ChangedTags =   2,  // The ExitTags on this exit have been changed.

    // Doors and other things that open.
    Openable =      100,    // Is this exit something that can open and close?
    Door =          101,    // Is this exit a door, specifically?
    SeeThrough =    102,    // Is this exit a window, or something else we can see through (e.g. bars)?
    Open =          103,    // Is this exit currently open?
    Lockable =      104,    // Can this exit be locked and unlocked?
    Locked =        105,    // Unlike open/closed, the default state is unlocked.
    Permalock =     106,    // For locks that have no key; they can never be unlocked.
    AwareOfLock =   107,    // The player is aware that this Link is locked.

    // Different types of doors (and openable things).
    Gate =          200,    // This 'door' should be called a gate.
    Window =        201,    // ... You know how this goes, I'm not gonna comment all of these.
    Grate =         202,

    // Tags regarding the map properties of this exit.
    MapNoFollow =   300,    // Don't follow this exit when drawing the automap.
    DoubleLength =  301,    // This exit is extra-long (passes through where one room would be).
    TripleLength =  302,    // As above, but passes through two rooms.
};

class Link
{
public:
    static LinkTag  parse_link_tag(std::string_view tag);   // Parses a string LinkTag name into a LinkTag enum.

                Link(); // Creates a new Link with default values.
    bool        changed() const;    // Checks if this Link has been modified.
    void        clear_tag(LinkTag the_tag, bool mark_delta = true); // Clears a LinkTag from this Link.
    void        clear_tags(std::list<LinkTag> tags_list, bool mark_delta = true);   // Clears multiple LinkTags at the same time.
    const std::string   door_name() const;  // Returns the name of the door (door, gate, etc.) on this Link, if any.
    uint32_t    get() const;    // Gets the Room linked to by this Link.)
    void        load_delta(FileReader* file);   // Loads the delta changes to this Link (should only be called from its parent Room).
    void        save_delta(FileWriter* file);   // Saves the delta changes to this Link (should only be called from its parent Room).
    void        set(uint32_t new_room, bool mark_delta = true);     // Sets this Link to point to a Room.
    void        set_tag(LinkTag the_tag, bool mark_delta = true);   // Sets a LinkTag on this Link.
    void        set_tags(std::list<LinkTag> tags_list, bool mark_delta = true); // Sets multiple LinkTags at the same time.
    bool        tag(LinkTag the_tag) const; // Checks if a LinkTag is set on this Link.

private:
    static constexpr uint32_t   LINK_DELTA_END =    0;  // Marks the end of the Link's delta changes.
    static constexpr uint32_t   LINK_DELTA_EXIT =   1;  // The linked exit on this Link has changed.
    static constexpr uint32_t   LINK_DELTA_TAGS =   2;  // The LinkTags on this Link have changed.

    static const std::map<std::string, LinkTag> tag_map_;   // Used during loading YAML data, to convert LinkTag text names into LinkTag enums.

    uint32_t    links_to_;      // The Room this Exit links to, or 0 for unlinked.
    std::set<LinkTag>   tags_;  // Any and all tags on this Link.
};

}   // namespace westgate
