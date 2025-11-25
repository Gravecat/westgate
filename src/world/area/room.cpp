// world/area/room.cpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <vector>

#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "util/text/hash.hpp"
#include "util/text/stringutils.hpp"
#include "world/area/automap.hpp"
#include "world/area/region.hpp"
#include "world/area/room.hpp"
#include "world/entity/mobile.hpp"
#include "world/entity/player.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"

using std::list;
using std::make_unique;
using std::map;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;
using westgate::terminal::print;

namespace westgate {

// Static map that converts a Direction enum into string names.
map<Direction, string> Room::direction_names_ = {
    { Direction::NORTH, "north" }, { Direction::NORTHEAST, "northeast" }, { Direction::EAST, "east" }, { Direction::SOUTHEAST, "southeast" },
    { Direction::SOUTH, "south" }, { Direction::SOUTHWEST, "southwest" }, { Direction::WEST, "west" }, { Direction::NORTHWEST, "northwest" },
    { Direction::UP, "up" }, { Direction::DOWN, "down" }, { Direction::NONE, "" }
};

// Used during loading YAML data, to convert RoomTag text names into RoomTag enums.
const std::map<std::string, RoomTag> Room::tag_map_ = { {"Explored", RoomTag::Explored }, { "Indoors", RoomTag::Indoors }, { "Windows", RoomTag::Windows },
    { "Streets", RoomTag::Streets }, { "Underground", RoomTag::Underground }, { "Trees", RoomTag::Trees }, { "AlwaysWinter", RoomTag::AlwaysWinter },
    { "AlwaysSpring", RoomTag::AlwaysSpring }, { "AlwaysSummer", RoomTag::AlwaysSummer }, { "AlwaysAutumn", RoomTag::AlwaysAutumn } };

// Creates a blank Room with default values and no ID.
Room::Room() : coords_{0,0,-10000}, desc_("Missing room description."), exits_{}, id_(0), map_char_("{M}?"), short_name_("undefined") { }

// Creates a Room with a specified ID.
Room::Room(const string& new_id) : Room()
{
    id_str_ = new_id;
    id_ = hash::murmur3(new_id);
}

// Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
void Room::add_entity(unique_ptr<Entity> entity)
{
    entity->set_parent_room(this);
    entities_.push_back(std::move(entity));
}

// Clears a RoomTag from this Room.
void Room::clear_tag(RoomTag the_tag, bool mark_delta)
{
    if (!(tags_.count(the_tag) > 0)) return;
    tags_.erase(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Clears multiple RoomTags at the same time.
void Room::clear_tags(list<RoomTag> tags_list, bool mark_delta)
{
    for (auto the_tag : tags_list)
        clear_tag(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Retrieves the coordinates of this Room.
const Vector3 Room::coords() const
{
    if (coords_ == Vector3(0,0,-10000)) throw runtime_error("Coordinates of room were never set! [" + id_str_ + "]");
    return coords_;
}

// Retrieves the description of this Room.
const string& Room::desc() const { return desc_; }

// Gets the string name of a Direction enum.
const string& Room::direction_name(Direction dir)
{
    auto result = direction_names_.find(dir);
    if (result == direction_names_.end())
    {
        core().nonfatal("Unable to parse direction enum.", Core::CORE_ERROR);
        return direction_names_.find(Direction::NONE)->second;
    }
    return result->second;
}

// Gets the Room linked in the specified direction, or nullptr if none is linked.
Room* Room::get_link(Direction dir)
{
    const uint8_t array_pos = static_cast<uint8_t>(dir) - 1;
    if (array_pos >= 10)
    {
        core().nonfatal("Attempt to retrieve invalid room link on " + id_str_ + " (" + to_string(array_pos) + ")", Core::CORE_ERROR);
        return nullptr;
    }
    if (!exits_[array_pos]) return nullptr;
    return world().find_room(exits_[array_pos]);
}

// Retrieves the hashed ID of this Room.
uint32_t Room::id() const { return id_; }

// Retrieves the string ID of this Room.
const string& Room::id_str() const { return id_str_; }

// Loads only the changes to this Room from a save file. Should only be called by a parent Region.
void Room::load_delta(FileReader* file)
{
    uint32_t delta_tag = 0;
    do
    {
        delta_tag = file->read_data<uint32_t>();
        switch(delta_tag)
        {
            case ROOM_DELTA_ENTITIES:
            {
                // Load any Entities in this Room.
                const size_t entity_count = file->read_data<size_t>();
                entities_.reserve(entity_count);
                for (size_t i = 0; i < entity_count; i++)
                {
                    EntityType type = file->read_data<EntityType>();
                    switch(type)
                    {
                        case EntityType::ENTITY: add_entity(make_unique<Entity>(file)); break;
                        case EntityType::MOBILE: add_entity(make_unique<Mobile>(file)); break;
                        case EntityType::PLAYER: add_entity(make_unique<Player>(file)); break;
                        default: throw runtime_error("Attempt to load unknown entity type: " + to_string(static_cast<int>(type)));
                    }
                }
                break;
            }

            case ROOM_DELTA_TAGS:
            {
                // Clear all existing tags, and load the full set of tags in from the save file.
                tags_.clear();
                size_t tag_count = file->read_data<size_t>();
                for (size_t i = 0; i < tag_count; i++)
                    set_tag(file->read_data<RoomTag>(), false);
                break;
            }

            case ROOM_DELTA_DESC:
            {
                // Update the room description.
                desc_ = file->read_string();
                break;
            }

            case ROOM_DELTA_EXITS:
            {
                // Clear the existing room exits, replace them with the save file data.
                for (int i = 0; i < 10; i++)
                    exits_[i] = file->read_data<uint32_t>();
                break;
            }

            case ROOM_DELTA_SHORT_NAME:
            {
                // Replace the short room name with the save file data.
                short_name_ = file->read_string();
                break;
            }

            case ROOM_DELTA_MAP_CHAR:
            {
                // Replace the map character with the save file data.
                map_char_ = file->read_string();
                break;
            }

            case ROOM_DELTA_END: break;
            default: FileReader::standard_error("Unrecognized delta tag in room data", delta_tag, 0, {id_str_});
        }
    } while(delta_tag != ROOM_DELTA_END);
}

// Look around you. Just look around you.
void Room::look() const
{
    const bool automap_enabled = !player().player_tag(PlayerTag::AutomapOff);
    const unsigned int term_width = terminal::get_width();
    const unsigned int minimap_width = (automap_enabled ? 11 : 0);
    const unsigned int desc_width = term_width - minimap_width;

    if (automap_enabled && !player().player_tag(PlayerTag::TutorialAutomap))
    {
        player().set_player_tag(PlayerTag::TutorialAutomap);
        print("{c}An automatically-generated map of the nearby area will be displayed next to the room description. If you wish to disable this feature, "
            "simply type: {C}automap off\n");
    }

    vector<string> room_desc = stringutils::ansi_vector_split(desc_, desc_width);

    vector<string> weather_desc = stringutils::ansi_vector_split("{K}" + world().time_weather().weather_desc(), desc_width);
    room_desc.insert(room_desc.end(), weather_desc.begin(), weather_desc.end());

    vector<string> exits_list;
    string exits_list_str;
    for (int i = 0; i < 10; i++)
    {
        const uint32_t exit = exits_[i];
        if (!exit) continue;
        string exit_name = "{C}" + direction_name(static_cast<Direction>(i + 1)) + "{c}";
        const Room* target_room = world().find_room(exit);
        if (target_room->tag(RoomTag::Explored)) exit_name += " (" + target_room->short_name() + ")";
        exits_list.push_back(exit_name);
    }
    if (exits_list.size()) exits_list_str = string("{c}There ") + (exits_list.size() > 1 ? "are " : "is ") + stringutils::number_to_text(exits_list.size()) +
        " obvious exit" + (exits_list.size() > 1 ? "s" : "") + ": " + stringutils::comma_list(exits_list, stringutils::CL_MODE_USE_AND) + ".";
    exits_list = stringutils::ansi_vector_split(exits_list_str, desc_width);
    room_desc.insert(room_desc.end(), exits_list.begin(), exits_list.end());

    vector<string> room_map;
    if (automap_enabled) room_map = world().automap().generate_map(this);
    bool desc_longer = room_desc.size() > room_map.size();
    for (size_t i = 0; i < (desc_longer ? room_map.size() : room_desc.size()); i++)
        room_map.at(i) += room_desc.at(i);
    if (desc_longer)
    {
        for (size_t i = room_map.size(); i < room_desc.size(); i++)
            room_map.push_back((automap_enabled ? "           " : "") + room_desc.at(i));
    }

    for (auto str : room_map)
        print(str);
}

// Retrieves the map character for this Room.
const std::string Room::map_char() const { return map_char_ + "{0}"; }

// Parses a string RoomTag name into a RoomTag enum.
RoomTag Room::parse_room_tag(const std::string &tag)
{
    auto result = tag_map_.find(tag);
    if (result == tag_map_.end()) throw runtime_error("Invalid RoomTag: " + tag);
    return result->second;
}

// Returns the ID of the Region this Room belongs to.
uint32_t Room::region() const
{ return world().find_room_region(id_); }

// Saves only the changes to this Room in a save file. Should only be called by a parent Region.
void Room::save_delta(FileWriter* file)
{
    // Check if anything has changed on this Room.
    const bool entities_exist = (entities_.size() > 0);
    const bool tags_changed = tag(RoomTag::ChangedTags);
    const bool desc_changed = tag(RoomTag::ChangedDesc);
    const bool exits_changed = tag(RoomTag::ChangedExits);
    const bool short_name_changed = tag(RoomTag::ChangedShortName);
    const bool map_char_changed = tag(RoomTag::ChangedMapChar);
    if (!(entities_exist || tags_changed || desc_changed || exits_changed || short_name_changed || map_char_changed)) return;

    // Write the save version for this Room, and the Room's ID.
    file->write_data<uint32_t>(Region::REGION_DELTA_ROOM);
    file->write_data<uint32_t>(ROOM_SAVE_VERSION);
    file->write_data<uint32_t>(id_);

    // Save any Entities in this Room.
    if (entities_exist)
    {
        file->write_data<uint32_t>(ROOM_DELTA_ENTITIES);
        file->write_data<size_t>(entities_.size());
        for (auto &entity : entities_)
            entity->save(file);
    }

    // If any tags have changed, write them all here.
    if (tags_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_TAGS);
        file->write_data<size_t>(tags_.size());
        for (auto &tag : tags_)
            file->write_data<RoomTag>(tag);
    }

    // If the room description has changed, add it here.
    if (desc_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_DESC);
        file->write_string(desc_);
    }

    // If any of the exits have changed, add them here.
    if (exits_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_EXITS);
        for (int i = 0; i < 10; i++)
            file->write_data<uint32_t>(exits_[i]);
    }

    // If the room's short name has changed, add it here.
    if (short_name_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_SHORT_NAME);
        file->write_string(short_name_);
    }

    // If the map character has changed, add it here.
    if (map_char_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_MAP_CHAR);
        file->write_string(map_char_);
    }

    // Mark the end of the changes.
    file->write_data<uint32_t>(ROOM_DELTA_END);
}

// Sets the coordinates of this room. Does not affect delta, as this should only ever be done when loading YAML.
void Room::set_coords(Vector3 new_coords)
{
    if (coords_ != Vector3(0,0,-10000)) throw runtime_error("Attempt to set coords of a room a second time! [" + id_str_ + "]");
    coords_ = new_coords;
}

// Sets the description of this Room.
void Room::set_desc(const string& new_desc, bool mark_delta)
{
    if (mark_delta) set_tag(RoomTag::ChangedDesc);
    if (!new_desc.size())
    {
        core().nonfatal("Attempt to set blank description on room (" + id_str_ + ")", Core::CORE_ERROR);
        desc_ = "Missing room description.";
    }
    else desc_ = new_desc;
}

// Sets an exit link from this Room to another.
void Room::set_exit(Direction dir, uint32_t new_exit, bool mark_delta)
{
    if (mark_delta) set_tag(RoomTag::ChangedExits);
    if (dir == Direction::NONE || dir > Direction::DOWN) throw runtime_error("Invalid direction on set_exit call (" + id_str_ + ")");
    exits_[static_cast<uint8_t>(dir) - 1] = new_exit;
}

// Sets the map character for this Room.
void Room::set_map_char(const std::string& new_char, bool mark_delta)
{
    if (mark_delta) set_tag(RoomTag::ChangedMapChar);
    map_char_ = new_char;
}

// Sets the short name of this Room.
void Room::set_short_name(const string& new_short_name, bool mark_delta)
{
    if (mark_delta) set_tag(RoomTag::ChangedShortName);
    if (!new_short_name.size())
    {
        core().nonfatal("Attempt to set blank name on room.", Core::CORE_ERROR);
        short_name_ = "undefined";
    }
    else short_name_ = new_short_name;
}

// Sets a RoomTag on this Room.
void Room::set_tag(RoomTag the_tag, bool mark_delta)
{
    if (tags_.count(the_tag) > 0) return;
    tags_.insert(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Sets multiple RoomTags at the same time.
void Room::set_tags(list<RoomTag> tags_list, bool mark_delta)
{
    for (auto the_tag : tags_list)
        set_tag(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Retrieves the short name of this Room.
const string& Room::short_name() const { return short_name_; }

// Checks if a RoomTag is set on this Room.
bool Room::tag(RoomTag the_tag) const { return (tags_.count(the_tag) > 0); }

// Transfers a specified Entity from this Room to a target Room.
void Room::transfer(Entity* entity_ptr, Room* room_ptr)
{
    // First, sanity checks. These should never happen, but could possibly occur as the result of mistakes in the code.
    if (room_ptr == this)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to itself.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer enity (" + entity_ptr->name() + ") from " + id_str_ + " to itself.", Core::CORE_ERROR);
        return;
    }
    if (!entity_ptr)
    {
        if (!room_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to " + room_ptr->id_str() + ".", Core::CORE_ERROR);
        return;
    }
    if (!room_ptr)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to null room.", Core::CORE_ERROR);
        return;
    }
    if (entity_ptr->parent_room() != this)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to " + room_ptr->id_str() +
            " while entity is not correctly parented to this room.", Core::CORE_ERROR);
        return;
    }

    // Try to determine which vector position houses the Entity being moved.
    uint32_t source_id = 0;
    bool source_found = false;
    for (size_t i = 0; i < entities_.size(); i++)
    {
        if (entities_.at(i).get() == entity_ptr)
        {
            source_id = i;
            source_found = true;
            break;
        }
    }
    if (!source_found)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to " + room_ptr->id_str() +
            ", while entity is not contained within the parent room.", Core::CORE_ERROR);
        return;
    }

    // We now know the position in the entities_ vector for the requested entity, and all the pointers appear to be in order. Let's transfer ownership...
    room_ptr->entities_.push_back(std::move(entities_[source_id]));
    // ...and immediately delete the now-invalid pointer from our own entities_ vector...
    entities_.erase(entities_.begin() + source_id);
    // ...and finally, update the Entity's parent Room pointer.
    entity_ptr->set_parent_room(room_ptr);
}

}   // namespace westgate
