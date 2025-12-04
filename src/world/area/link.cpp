// world/area/link.cpp -- A Link is a connection between two Rooms. It can have its own special properties, such as being a door.

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

#include "util/filex.hpp"
#include "world/area/link.hpp"

using std::runtime_error;
using std::to_string;

namespace westgate {

// Used during loading YAML data, to convert LinkTag text names into LinkTag enums.
const std::map<std::string, LinkTag> Link::tag_map_ = { { "Openable", LinkTag::Openable }, { "Door", LinkTag::Door }, { "SeeThrough", LinkTag::SeeThrough },
    { "Open", LinkTag::Open }, { "Gate", LinkTag::Gate }, { "Window", LinkTag::Window }, { "Lockable", LinkTag:: Lockable }, { "Locked", LinkTag::Locked },
    { "Permalock", LinkTag::Permalock }, { "AwareOfLock", LinkTag::AwareOfLock }, { "Grate", LinkTag::Grate }, { "MapNoFollow", LinkTag::MapNoFollow },
    { "DoubleLength", LinkTag::DoubleLength }, { "TripleLength", LinkTag::TripleLength } };

// Creates a new Link with default values.
Link::Link() : links_to_(0) { }

// Checks if this Link has been modified.
bool Link::changed() const
{ return (tag(LinkTag::ChangedLink) || tag(LinkTag::ChangedTags)); }

// Clears a LinkTag from this Link.
void Link::clear_tag(LinkTag the_tag, bool mark_delta)
{
    if (!(tags_.count(the_tag) > 0)) return;
    tags_.erase(the_tag);
    if (mark_delta) set_tag(LinkTag::ChangedTags, false);
}

// Clears multiple LinkTags at the same time.
void Link::clear_tags(std::list<LinkTag> tags_list, bool mark_delta)
{
    for (auto the_tag : tags_list)
        clear_tag(the_tag);
    if (mark_delta) set_tag(LinkTag::ChangedTags, false);
}

// Returns the name of the door (door, gate, etc.) on this Link, if any.
const std::string Link::door_name() const
{
    if (!tag(LinkTag::Openable)) return "";
    if (tag(LinkTag::Gate)) return "gate";
    if (tag(LinkTag::Window)) return "window";
    if (tag(LinkTag::Grate)) return "grate";
    return "door";
}

// Gets the Room linked to by this Link.
uint32_t Link::get() const { return links_to_; }

// Loads the delta changes to this Link (should only be called from its parent Room).
void Link::load_delta(FileReader* file)
{
    uint32_t delta_tag = 0;
    while(true)
    {
        delta_tag = file->read_data<uint32_t>();
        switch(delta_tag)
        {
            case LINK_DELTA_END: return;
            case LINK_DELTA_EXIT: links_to_ = file->read_data<uint32_t>(); break;
            case LINK_DELTA_TAGS:
            {
                size_t tag_count = file->read_data<size_t>();
                for (size_t i = 0; i < tag_count; i++)
                    set_tag(file->read_data<LinkTag>(), false);
                break;
            }
            default: throw runtime_error("Unknown Link tag in save data [" + to_string(delta_tag) + "]");
        }
    }
}

// Parses a string LinkTag name into a LinkTag enum.
LinkTag Link::parse_link_tag(const std::string &tag)
{
    auto result = tag_map_.find(tag);
    if (result == tag_map_.end()) throw runtime_error("Invalid LinkTag: " + tag);
    return result->second;
}

// Saves the delta changes to this Link (should only be called from its parent Room).
void Link::save_delta(FileWriter* file)
{
    if (tag(LinkTag::ChangedLink))
    {
        file->write_data<uint32_t>(LINK_DELTA_EXIT);
        file->write_data<uint32_t>(links_to_);
    }
    if (tag(LinkTag::ChangedTags))
    {
        file->write_data<uint32_t>(LINK_DELTA_TAGS);
        file->write_data<size_t>(tags_.size());
        for (auto &tag : tags_)
            file->write_data<LinkTag>(tag);
    }
    file->write_data<uint32_t>(LINK_DELTA_END);
}

// Sets this Link to point to a Room.
void Link::set(uint32_t new_room, bool mark_delta)
{
    links_to_ = new_room;
    if (mark_delta) set_tag(LinkTag::ChangedLink);
}

// Sets a LinkTag on this Link.
void Link::set_tag(LinkTag the_tag, bool mark_delta)
{
    if (tags_.count(the_tag) > 0) return;
    tags_.insert(the_tag);
    if (mark_delta) set_tag(LinkTag::ChangedTags, false);
}

// Sets multiple LinkTags at the same time.
void Link::set_tags(std::list<LinkTag> tags_list, bool mark_delta)
{
    for (auto the_tag : tags_list)
        set_tag(the_tag);
    if (mark_delta) set_tag(LinkTag::ChangedTags, false);
}

// Checks if a LinkTag is set on this Link.
bool Link::tag(LinkTag the_tag) const { return (tags_.count(the_tag) > 0); }

}   // namespace westgate
